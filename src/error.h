#ifndef ERROR_H_
#define ERROR_H_

#include <QMessageBox>
#include <QDialog>

void error(QString message);
void error(QString message, const char* filename, int line);

#endif /*ERROR_H_*/
