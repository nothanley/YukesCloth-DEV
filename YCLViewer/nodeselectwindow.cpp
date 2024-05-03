#include "nodeselectwindow.h"
#include "ui_nodeselectwindow.h"
#include "treelabeldelegate.h"
#include <YukesCloth>
#include <QFileInfo>

NodeSelectWindow::NodeSelectWindow(std::shared_ptr<CGameCloth>& pSimObject, const StTag* pTargetTag, QWidget *parent, const char *fileName) :
    QDialog(parent),
    ui(new Ui::NodeSelectWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("Add New Node");
    this->m_clothObj = pSimObject;
    this->m_pParentTag = pTargetTag;
    this->m_sFileName = fileName;
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->listWidget->setItemDelegate(new TreeLabelDelegate);

    this->setAttribute(Qt::WA_DeleteOnClose);
    NodeSelectWindow::SetupListWidget();
    NodeSelectWindow::UpdateGUILabels();

}

NodeSelectWindow::~NodeSelectWindow()
{
    emit interfaceClose();
    delete ui;
}

static const int GetSimMeshCount(const StTag* pTagHead)
{
    int numSims = 0;
    for (auto& child : pTagHead->children)
        numSims += (child->eType == 0x5) ? 1 : 0;

    return numSims;
}

QString
GetFileString(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}

void
NodeSelectWindow::UpdateGUILabels(){
    auto root = m_clothObj->getRootTag();
    int numSimMesh = GetSimMeshCount(root);
    int numNodes = root->children.size();

    QString fileName = GetFileString(QString::fromStdString(m_sFileName));
    ui->fileLabel->setText(fileName);
    ui->meshLabel->setText("Mesh Count: " + QString::number(numSimMesh));
    ui->nodeLabel->setText( QString::number(numNodes) + " Node(s)");
}

QString
GetNodeName(const StTag* pSourceTag){
    QString nodeText = QString::fromStdString(yclutils::GetNodeName(pSourceTag->eType));

    if (pSourceTag->eType == /* SIMMESH */ 0x5 || pSourceTag->eType == 0x1B){
        nodeText += " (" + QString::fromStdString(pSourceTag->pSimMesh->sModelName)
                + " : "+ QString::fromStdString(pSourceTag->pSimMesh->sObjName) + ")";
    }
    else if(pSourceTag->eType == /* STRING */ 0x18){
        nodeText += " (" + QString::fromStdString(pSourceTag->sTagName) + ")";
    }
    return nodeText;
}

void
NodeSelectWindow::SetupListWidget(){
    ui->listWidget->clear();
    m_tagPalette = m_clothObj->getRootTag()->children;

    for (int i = 0; i < m_tagPalette.size(); i++)
    {
        StTag* pTag = m_tagPalette[i];
        QString listStr = QString::number(i) + ": " + GetNodeName(pTag);

        if (pTag->eType == 0x5)
        {
            ui->listWidget->addItem(listStr);

            QListWidgetItem* item = ui->listWidget->item(ui->listWidget->count()-1);
            item->setData(Qt::UserRole,pTag->eType);
        }
    }
}

void NodeSelectWindow::on_buttonBox_rejected()
{
    this->close();
}

bool hasListItem(const QListWidgetItem* sItem, const std::vector<QListWidgetItem*>& items)
{
    for (auto& item : items) {
        if (item == sItem)
            return true;
    }
    return false;
}


std::vector<QListWidgetItem*>
NodeSelectWindow::SortYCLNodes( const std::vector<QListWidgetItem*>& items )
{
    std::vector<QListWidgetItem*> sortedMeshes;

    /* List all source meshes */
     for (auto& item : items){
         int nodeIdx = item->text().split(":").first().toInt();
         StTag* sourceTag = m_tagPalette.at(nodeIdx);
         StSimMesh* pSimMesh = sourceTag->pSimMesh;
         bool isTargetMesh = pSimMesh->target.indices.size() == 0;

         if (!isTargetMesh)
             sortedMeshes.push_back(item);
     }

     /* Append remaining nodes */
     for (auto& item : items){
         if ( !hasListItem(item, sortedMeshes) )
             sortedMeshes.push_back(item);
     }


    return sortedMeshes;
}

std::vector<QListWidgetItem*>
NodeSelectWindow::GetAllSelectedMeshItems(){
    std::vector<QListWidgetItem*> items;

    for (int i = 0; i < ui->listWidget->count(); i++){
        QListWidgetItem* item = ui->listWidget->item(i);
        bool isQueriedItem = item->isSelected();
        if (isQueriedItem){ items.push_back(item); }
    }

    std::vector<QListWidgetItem*> sortedItems = SortYCLNodes(items);
    return sortedItems;
}

void NodeSelectWindow::on_buttonBox_accepted()
{
    /* Gets a list of all selected & sorted mesh items */
    std::vector<QListWidgetItem*> listItems = GetAllSelectedMeshItems();

    /* Appends to target hierarchy */
    for (auto& item : listItems){
        int nodeIdx = item->text().split(":").first().toInt();
        StTag* sourceTag = m_tagPalette.at(nodeIdx);
        emit addNodeToItem(sourceTag);
    }

    emit validateParent();

    /* Clear MEM!! */
    this->close();
}




void NodeSelectWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    on_buttonBox_accepted();
}


void NodeSelectWindow::on_listWidget_itemClicked(QListWidgetItem *item){}

void NodeSelectWindow::on_listWidget_itemSelectionChanged()
{
    int selectionCount = ui->listWidget->selectedItems().size();
    ui->lineCount->setText( "Items Selected: " + QString::number(selectionCount) );
}











