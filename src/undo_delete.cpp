#include "undo_delete.h"

RemoveItemCommand::RemoveItemCommand(DrawingCanvas *canvas, QUndoCommand *parent)
     : QUndoCommand(parent)
 {
     myCanvas = canvas;
     myList = canvas->selectedItems();
     setText(QObject::tr("Remove %1").arg(myList.size()>1 ? "items" : "item"));
 }

 void RemoveItemCommand::undo()
 {
	 // TODO also remove from atomslist, and add back in later
	 foreach(QGraphicsItem *item, myList){
	     myCanvas->addItem(item);		 
	 }
	 myCanvas->update();
 }

 void RemoveItemCommand::redo()
 {
	 foreach(QGraphicsItem *item, myList){
	     myCanvas->removeItem(item);		 
	 }
	 myCanvas->update();
 }
