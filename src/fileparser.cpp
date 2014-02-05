#include "fileparser.h"

using namespace std;

FileParser::FileParser(QString instring):
		myUnits(Angstrom),
		currentGeometry(0)
{
	if(instring != 0){
		QDir* dir = new QDir(instring);
		myFileName = dir->absolutePath();
	}
}

FileParser::~FileParser()
{

}

void FileParser::setFileName(const QString name)
{
	QDir* dir = new QDir(name);
	myFileName = dir->absolutePath();
}

void FileParser::determineFileType()
{
	if(myFileName.endsWith(".chmvp")) return;

	fileType = UNKNOWN;

	if (myFileName.contains("file11", Qt::CaseInsensitive))
		fileType = FILE11;
	else if (myFileName.contains("xyz", Qt::CaseInsensitive))
		fileType = XYZ;
	else {
		// Go through the file and determine which program the output is from.
		std::string tempS;
		QString tempString;
		while (1) {
			getline(infile, tempS);
			tempString = tempS.c_str();
			if (infile.eof())
				break;
			if (tempString.contains("optking", Qt::CaseInsensitive) ||
				tempString.contains("psi 3", Qt::CaseInsensitive)) {
				fileType = PSI3;
				break;
			}
			else if (tempString.contains("gamess", Qt::CaseInsensitive)) {
				fileType = GAMESS;
				break;
			}
			else if (tempString.contains("PROGRAM SYSTEM MOLPRO", Qt::CaseInsensitive) ||
					 tempString.contains("Running default procedure", Qt::CaseInsensitive)) {
				// The log file doesn't produce a header, so I'll assume that this is always present..
				fileType = MOLPRO;
				break;
			}
			else if (tempString.contains("aces2", Qt::CaseInsensitive) ||
					 tempString.contains("cfour", Qt::CaseInsensitive)) {
				fileType = ACES2;
				break;
			}
			else if (tempString.contains("Northwest Computational Chemistry Package", Qt::CaseInsensitive)) {
				fileType = NWCHEM;
				break;
			}
			else if (tempString.contains("O   R   C   A", Qt::CaseInsensitive)) {
				fileType = ORCA;
				break;
			}
			else if (tempString.contains("Q-Chem, Version 3.0", Qt::CaseInsensitive)
				|| tempString.contains("Q-Chem, Version 3.1", Qt::CaseInsensitive)) {
				fileType = QCHEM3_1;
				break;
			}
		}
	}

	infile.seekg(ios_base::beg);
}

void FileParser::readFile()
{
	if(myFileName.isEmpty() || myFileName.endsWith(".chmvp")) return;

	infile.open(myFileName.toLatin1());
	if (!infile) {
		QString errorMessage = "Unable to open " + myFileName + " for reading";
		error(errorMessage, __FILE__, __LINE__);
	}

	determineFileType();
	foreach(Molecule *molecule, myMoleculeList){
		delete molecule;
	}
	myMoleculeList.clear();

	switch(fileType){
				case XYZ:
#ifdef QT_DEBUG
		std::cout << "Reading in XYZ." << std::endl;
#endif
		readXYZ();
		break;
				case FILE11:
#ifdef QT_DEBUG
		std::cout << "Reading in File11." << std::endl;
#endif
		readFile11();
		break;
	case ACES2:
#ifdef QT_DEBUG
		std::cout << "Reading in ACES2." << std::endl;
#endif
		readACES2();
		break;
	case PSI3:
#ifdef QT_DEBUG
		std::cout << "Reading in Psi3." << std::endl;
#endif
		readPsi3();
		break;
	case ORCA:
#ifdef QT_DEBUG
		std::cout << "Reading in ORCA." << std::endl;
#endif
		readORCA();
		break;
	case NWCHEM:
#ifdef QT_DEBUG
		std::cout << "Reading in NWChem." << std::endl;
#endif
		readNWChem();
		break;
	case MOLPRO:
#ifdef QT_DEBUG
		std::cout << "Reading in Molpro." << std::endl;
#endif
		readMolpro();
		break;
	case GAMESS:
		readGamess();
		break;
		// case ACES2:
		//     readACES2();
		//     break;
	case QCHEM3_1:
#ifdef QT_DEBUG
		std::cout << "Reading in Qchem3.1." << std::endl;
#endif
		readQchem31();
		break;
				default:
		QString errorMessage = "Unknown file type for " + myFileName;
		error(errorMessage, __FILE__, __LINE__);
#ifdef QT_DEBUG
		std::cout << "Unknown file type." << std::endl;
#endif
		;
	}
	if (myMoleculeList.size()) {
		currentGeometry = myMoleculeList.size() - 1;
        // This should (hopefully) account for the x, -y coordinate system used for the screen
        myMoleculeList[currentGeometry]->invert_y();
	}
	infile.close();
}

void FileParser::serialize(QXmlStreamWriter* writer)
{
	writer->writeStartElement("FileParser");
	writer->writeAttribute("units", QString("%1").arg(myUnits));
	writer->writeAttribute("step", QString("%1").arg(currentGeometry));
	writer->writeAttribute("items", QString("%1").arg(myMoleculeList.size()));
	foreach(Molecule* m, myMoleculeList)
		m->serialize(writer);
	writer->writeEndElement();
}

FileParser* FileParser::deserialize(QXmlStreamReader* reader)
{
	reader->readNextStartElement();
	Q_ASSERT(reader->isStartElement() && reader->name() == "FileParser");

	FileParser* parser = new FileParser(NULL);
	parser->myFileName = "";
	parser->myUnits = (reader->attributes().value("units").toString().toInt() == 0) ? Angstrom : Bohr;
	parser->currentGeometry = reader->attributes().value("step").toString().toInt();
	int size = reader->attributes().value("items").toString().toInt();
	for(int i = 0; i < size; i++)
		parser->myMoleculeList.append(Molecule::deserialize(reader));
	reader->skipCurrentElement();
	return parser;
}
