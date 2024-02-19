#ifndef DEFWIDGETITEM_H
#define DEFWIDGETITEM_H

#include <QTreeWidgetItem>
#include <QMessageBox>
#pragma once

class StTag;
class DefWidgetItem : public QTreeWidgetItem
{
public:
    virtual void item() const {}
    DefWidgetItem(QTreeWidget *treeview, int type = Type)  : QTreeWidgetItem(treeview,type) {};
    DefWidgetItem(QTreeWidgetItem *parent, int type = Type) : QTreeWidgetItem(parent,type) {};

    void setItemTag(StTag* node) { m_pTag = node; }
    StTag* getItemTag(){return this->m_pTag;}

private:
    StTag* m_pTag = nullptr;

};


#endif // DEFWIDGETITEM_H


