#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "defwidgetitem.h"
#include "nodeselectwindow.h"
#include "treelabeldelegate.h"
#include "clothjson.h"
#include "YukesCloth"
#include <Encoder/Import/SimMeshJson.h>
#include "Encoder/clothsave.h"

#ifdef DEBUG_EDITOR
#include <QHBoxLayout>
#include <QTextEdit>
#include <QScrollBar>
#endif


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QStringList appVersionList{ QString::number(MAJOR_VER), QString::number(MINOR_VER), QString::number(REVIS_VER) };
    QString versionString = appVersionList.join(".");
    this->setWindowTitle("YCL Editor v" + appVersionList.join(".") );
    this->m_clothObj = nullptr;
    ui->versionLabel->setText("v" + versionString);

#ifdef DEBUG_EDITOR
    this->setWindowTitle("YCL Editor v" + appVersionList.join(".") + " - Debug");
    InitializeDebugWidgets();
#endif

}

MainWindow::~MainWindow()
{
    delete ui;
}

#ifdef DEBUG_EDITOR
void
MainWindow::InitializeDebugWidgets(){
    QFrame* frame = new QFrame(this);
    frame->setLayout(new QHBoxLayout(frame));

    m_DbgTextWidget = new QTextEdit(frame);
    m_DbgConsoleLog = new QTextEdit(frame);
    m_DbgTextWidget->setStyleSheet("QTextEdit{ padding-left: 30px; background-color: black; }");
    m_DbgConsoleLog->setStyleSheet("QTextEdit{ padding-left: 5px; background-color: black; }");
    ui->centralwidget->layout()->addWidget(frame);
    frame->layout()->addWidget(m_DbgTextWidget);
    frame->layout()->addWidget(m_DbgConsoleLog);
    frame->setFixedHeight(200);

    m_DbgTextWidget->setMaximumHeight(200);
    m_DbgConsoleLog->setMaximumHeight(200);
    m_DbgConsoleLog->setReadOnly(true);
    m_DbgTextWidget->setReadOnly(true);
    this->resize(width(),height()+120);
}
#endif

QString
getFileName(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.fileName();
}


QString
getDirName(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.absolutePath();
}

void
MainWindow::on_OpenFile_clicked()
{
    QString openPath = (m_sYclFilePath == "") ? m_sExplorerPath : getDirName(m_sYclFilePath);
    this->m_sYclFilePath = QFileDialog::getOpenFileName(this, tr("Open .ycl file"),
                                                         openPath,
                                                         tr("Yukes ClothSim Container (*.ycl)") );
    if (m_sYclFilePath == "") return;
    OpenYukesClothFile(m_sYclFilePath);
}

static const void GetNodeCount(const uint32_t& target, const StTag* pNode, uint32_t& totalNodes ){
    if (pNode->eType == target)
        totalNodes++;

    for (auto& child : pNode->children){
        GetNodeCount(target, child, totalNodes);
    }
}

void
MainWindow::UpdateStatLabels(){

    uint32_t numMeshes = 0;
    uint32_t numLines = 0;
    uint32_t numCols = 0;

    const StTag* pRootNode = m_clothObj->getRootTag();
    GetNodeCount(enTagType_SimMesh, pRootNode, numMeshes);
    GetNodeCount(enTagType_SimLine, pRootNode, numLines);
    GetNodeCount(enTagType_Capsule_Standard, pRootNode, numCols);
    GetNodeCount(enTagType_Capsule_Tapered, pRootNode, numCols);


    ui->nodeLabel->setText(QString::number(pRootNode->children.size()) + " node(s)");
    ui->pathLabel->setText( getFileName(m_sYclFilePath) );
    ui->meshTotalLabel->setText("SimMesh Total: " + QString::number(numMeshes));
    ui->lineTotalLabel->setText("SimLine Total: " + QString::number(numLines));
    ui->colTotalLabel->setText("Collision Total: " + QString::number(numCols));
}

void
MainWindow::OpenYukesClothFile(const QString& filePath){
    m_sYclFilePath = filePath;
    CClothContainer* clothFile;

    try{
        clothFile = new CClothContainer(filePath.toStdString().c_str());
        clothFile->open();
    }
    catch(...){
        QMessageBox::warning(this,"Load Error","Cannot load invalid .ycl file.");
        return;
    }

    m_clothObj = clothFile->getClothObj();
    const StTag* pRootNode = m_clothObj->getRootTag();
    PopulateTreeWidget(pRootNode);
    UpdateStatLabels();
}

void
MainWindow::OpenAppDataYCLFile(const QString& filePath){
    CClothContainer* clothFile;
    try{
        clothFile = new CClothContainer(filePath.toStdString().c_str());
        clothFile->open();
    }
    catch(...){
        return;
    }

    m_clothObj = clothFile->getClothObj();
    const StTag* pRootNode = m_clothObj->getRootTag();
    PopulateTreeWidget(pRootNode);
    UpdateStatLabels();
}

void
MainWindow::on_actionOpen_ycl_file_triggered()
{
    on_OpenFile_clicked();
}

void
MainWindow::PopulateTreeWidget(const StTag* pRootNode){
    ui->treeWidget->clear();

    for (const auto child : pRootNode->children)
        appendNodeToView(child, nullptr);

    if (isExpanded)
    {
        ui->treeWidget->expandAll();
    }
    else{
        ui->treeWidget->collapseAll();
    }

    TreeLabelDelegate* itemDelegate = new TreeLabelDelegate;
    ui->treeWidget->setItemDelegate(itemDelegate);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void
SetNodeText(const StTag* pSourceTag, DefWidgetItem* pTreeItem){
    /* Format Text */
    QString sNodeType = QString::fromStdString(yclutils::GetNodeName(pSourceTag->eType));
    pTreeItem->setText(0,sNodeType);

    if (pSourceTag->eType == /* SIMMESH */ 0x5 || pSourceTag->eType == 0x1B){
        QString text = pTreeItem->text(0);
        text += " (" + QString::fromStdString(pSourceTag->pSimMesh->sModelName)
                + " : "+ QString::fromStdString(pSourceTag->pSimMesh->sObjName) + ")";
        pTreeItem->setText(0,text);
    }
    else if(pSourceTag->eType == /* STRING */ 0x18){
        QString text = pTreeItem->text(0);
        text += " (" + QString::fromStdString(pSourceTag->sTagName) + ")";
        pTreeItem->setText(0,text);
    }
}

void
MainWindow::appendNodeToView(StTag* pSourceTag, DefWidgetItem* pParentItem){

    /* Populate items to tree */
    DefWidgetItem* pTreeItem = (pParentItem) ? new DefWidgetItem(pParentItem) : new DefWidgetItem(ui->treeWidget);
    pTreeItem->setItemTag(pSourceTag);
    pTreeItem->setData(0,Qt::UserRole, pSourceTag->eType);

    /* Format Text */
    SetNodeText(pSourceTag,pTreeItem);

    for (const auto child : pSourceTag->children)
        appendNodeToView(child, pTreeItem);

}

QString
intToHexQString(int value) {
    // Convert the integer to a hexadecimal string
    QString hexString = QString::number(value, 16).toUpper();

    // Ensure that the string has exactly two characters by padding with a leading zero if necessary
    if (hexString.length() < 2)
        hexString.prepend('0');

    return hexString;
}

void
AppendTableItem(QTableWidget* pTableWidget, const QString& text, int colIdx = 0){
    int numRows = pTableWidget->rowCount();
    pTableWidget->setRowCount(numRows+1);
    pTableWidget->setItem(numRows,colIdx,new QTableWidgetItem(text));
}

void
AppendTableItem(QTableWidget* pTableWidget, const uint32_t& value, int colIdx = 0){
    int numRows = pTableWidget->rowCount();
    pTableWidget->setRowCount(numRows+1);
    pTableWidget->setItem(numRows,colIdx,new QTableWidgetItem(QString::number(value)));
}

void
ClearTable(QTableWidget* pTable){
    pTable->clear();
    pTable->setColumnCount(0);
    pTable->setRowCount(0);
}

void removeElementAtIndex(std::vector<StTag*>& vec, size_t index) {
    if (index < vec.size()) { // Ensure the index is valid
        vec.erase(vec.begin() + index); // Remove the element at the specified index
    }
}

int
GetParentChildNodeIndex(StTag* pSourceTag){
    if (!pSourceTag->pParent) return -1;
    std::vector<StTag*> pTags = pSourceTag->pParent->children;
    for (int i = 0; i < pTags.size(); i++)
    {
        StTag* node = pTags.at(i);
        if (node->eType         == pSourceTag->eType &&
            node->sSize         == pSourceTag->sSize &&
            node->sTagName      == pSourceTag->sTagName &&
            node->streamPointer == pSourceTag->streamPointer)
        {
            return i;
        }
    }
    return -1;
}

QStringList
GetTableHeaderLabels(QTableWidget* pTable){
    QStringList labels;

    int labelCount = pTable->verticalHeader()->count();
    for (int i = 0; i < labelCount; i++){
        QString label = pTable->verticalHeaderItem(i)->text();
        labels.push_back(label);
    }

    return labels;
}

void DisplayMeshTableData(StSimMesh* pSimMesh, QTableWidget* pTable){
    if (!pSimMesh) return;
    QStringList meshHeaders = GetTableHeaderLabels(pTable) + QStringList{
                              "Mesh Name","Mesh Vertex Count",
                              "Sim Vertex Count","Constraints",
                              "Is Sim Line", "Nodes", "Display Object",
                              "Calc Normals", "Iterations"    };

    AppendTableItem(pTable, QString::fromStdString(pSimMesh->sObjName));
    AppendTableItem(pTable, pSimMesh->sObjVerts.size());
    AppendTableItem(pTable, pSimMesh->sSimVtxCount);
    AppendTableItem(pTable, pSimMesh->constraints.size());
    AppendTableItem(pTable, pSimMesh->bIsSimLine);
    AppendTableItem(pTable, pSimMesh->nodePalette.size());
    AppendTableItem(pTable, pSimMesh->bDispObject);
    AppendTableItem(pTable, pSimMesh->bCalcNormal);
    AppendTableItem(pTable, pSimMesh->sIterationCount);
    pTable->setVerticalHeaderLabels(meshHeaders );
}

void
MainWindow::PopulateTableWidget(StTag* pNode, QTableWidget* pTable){
    ClearTable(pTable);
    QStringList headers = {"Tag Type", "Tag Size",
                           "Tag Address","Children"};
    pTable->setColumnCount(1);

    AppendTableItem(pTable, pNode->eType);
    AppendTableItem(pTable, pNode->sSize);
    AppendTableItem(pTable, pNode->streamPointer);
    AppendTableItem(pTable, pNode->children.size());

    pTable->verticalHeader()->setVisible(true);
    pTable->horizontalHeader()->setVisible(false);
    pTable->setVerticalHeaderLabels(headers);

    /* Add Custom Tag Table Values */
    DisplayMeshTableData(pNode->pSimMesh, pTable);
    pTable->verticalHeader()->setAttribute(Qt::WA_TransparentForMouseEvents);
}

#ifdef DEBUG_EDITOR
void
WriteBufferHexToDbgConsole(StTag* pSourceTag, QTextEdit* pWidget){
    QString sDataHexString;

    for (int i = 0; i < pSourceTag->data.size(); i++){
        auto byte = pSourceTag->data.at(i);
        sDataHexString += (i % 16 == 0) ?
                    "\n" +intToHexQString(byte) :
                    " " +intToHexQString(byte);
    }
    pWidget->setText(sDataHexString);
}

void
UpdateDbgConsoleLog(StTag* pTag, QTextEdit* pConsole){
    QString text = pConsole->toPlainText() + "\n\n";
    text += "[Load TagBuffer 0x" + QString::number(pTag->eType,16) + " ]" + "\n";
    text += "Data Size: " + QString::number(pTag->data.size()) + " bytes" + "\n";
    text += "Address: " + QString::number(pTag->streamPointer) + "\n";
    pConsole->setText(text);
    pConsole->verticalScrollBar()->setValue(
                pConsole->verticalScrollBar()->maximum() );
}
#endif

void
MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    DefWidgetItem* pTreeItem = (DefWidgetItem*)item;
    StTag* pSourceTag = pTreeItem->getItemTag();

    ui->ItemAddress->clear();
    ui->ItemName->clear();
    ui->ItemName->setText( yclutils::GetNodeName(pSourceTag->eType).c_str() );
    ui->ItemAddress->setText( "0x"+QString::number(pSourceTag->streamPointer-0xC,16).toUpper() );
    PopulateTableWidget(pSourceTag, ui->tableWidget);

#ifdef DEBUG_EDITOR
    WriteBufferHexToDbgConsole(pSourceTag,m_DbgTextWidget);
    UpdateDbgConsoleLog(pSourceTag,m_DbgConsoleLog);
#endif
}

void
MainWindow::on_actionExpand_Collapse_triggered()
{
    if (isExpanded){
        ui->treeWidget->collapseAll();
    }
    else{
        ui->treeWidget->expandAll();
    }

    isExpanded = !isExpanded;
}


void
MainWindow::on_actionExit_triggered()
{
    QApplication::exit(0);
}


void
MainWindow::on_SaveButton_clicked()
{
    if (!this->m_clothObj){
        QMessageBox::warning(this,"File Error", "Please load .ycl to save contents.");
        return; }

    if (m_clothObj->getRootTag()->children.size() == 0){
        QMessageBox::warning(this,"File Error", "YCL is missing a tag hierarchy.");
        return;}

    QString savePath = QFileDialog::getSaveFileName(this, tr("Save .ycl file as"),
                                                         this->m_sYclFilePath,
                                                         tr("Yukes ClothSim Container (*.ycl)") );
    if (savePath == "") return;
    if (!savePath.endsWith(".ycl"))
        savePath += ".ycl";

    CClothSave::SaveToDisk(savePath.toStdString().c_str(), m_clothObj);
    QMessageBox::information(this,"Save Complete", "File saved to: " + savePath);
}


void
MainWindow::on_actionSave_File_triggered()
{
    on_SaveButton_clicked();
}

void
MainWindow::ClearSelectionWindow()
{
    this->m_pSelectionWindow = nullptr;
}

static void AddStringToVector(std::vector<std::string>& vec, const std::string& target) {
    for (const auto& str : vec)
        if (str == target) return;

    vec.push_back(target);
}

int
FindNodeIndex(const SimNode& sTarget, std::vector<SimNode> nodeTable)
{
    for (int i = 0; i < nodeTable.size(); i++) {
        SimNode nodeItem = nodeTable.at(i);

        if (sTarget.name == nodeItem.name &&
            sTarget.vecf == nodeItem.vecf) {
            return i;
        }
    }

    return -1;
}

int
MainWindow::GetTagIndex_SimMesh(StTag* pTag){
    StSimMesh* pDestMesh = pTag->pSimMesh;
    const StTag* sceneRoot = m_clothObj->getRootTag();

    for (int i = 0; i < sceneRoot->children.size(); i++)
    {
        StTag* branch = sceneRoot->children.at(i);

        /* Check if mesh name is equal to target */
        if(branch->pSimMesh)
        {
            std::string branchMeshName = branch->pSimMesh->sObjName;
            if (branchMeshName == pDestMesh->sObjName)
                return i;
        }

    }
    return -1;
}

bool
isMissingSourceMesh(StSimMesh* pSimMesh, StTag* pTagHead){
    for (auto& item : pTagHead->children)
    {
        StSimMesh* pTreeMesh = item->pSimMesh;
        if (pTreeMesh){
            if (pTreeMesh->sObjName == pSimMesh->target.source)
                return false;
            if (pTreeMesh->sObjName == pSimMesh->sObjName)
                return true;
        }
    }
    return true;
}

bool
MainWindow::AddNodeToTreeTag(StTag* newTag)
{
    if (!newTag->pSimMesh) return false;

    /* Verify target mesh isn't already pre-existing */
    StSimMesh* pSimMesh = newTag->pSimMesh;
    QString meshName = QString::fromStdString(pSimMesh->sObjName);
    int meshIndex = GetTagIndex_SimMesh(newTag);

    if (meshIndex != -1)
    {
        QMessageBox::warning(this,"Merge Error","Mesh Item: \"" + meshName
                             + "\" already exists in tree. Please remove original entry first." );
        return false;
    }

    /* Append to node palette if doesn't exists */
    for (auto& node : pSimMesh->skin.nodePalette)
    {
        int nodeIdx = FindNodeIndex(node, m_clothObj->getNodes());
        if (nodeIdx == -1)
            m_clothObj->addNode(node);
    }

    /* Append mesh names to string table */
    m_clothObj->addString( newTag->pSimMesh->sModelName );
    m_clothObj->addString( newTag->pSimMesh->sObjName );
    m_clothObj->addRootChild(newTag); /* Add node to parent children */

    /* Reload Hierarchy */
    PopulateTreeWidget(m_clothObj->getRootTag());
    UpdateStatLabels();
    return true;
}

void
MainWindow::on_addnodebutton_clicked()
{
    if (this->m_pSelectionWindow || !m_clothObj) return;
    const StTag* pUserTag = m_clothObj->getRootTag();
    if (!pUserTag) return;

    /* Get source file path */
    QString sourceFile = QFileDialog::getOpenFileName(this, tr("Open .ycl file"),
                                                         this->m_sExplorerPath,
                                                         tr("Yukes ClothSim Container (*.ycl)") );
    if (sourceFile == "") return;


    try{
        CClothContainer yclFile(sourceFile.toStdString().c_str());
        yclFile.open();
        this->m_pSelectionWindow = new NodeSelectWindow( yclFile.getClothObj(), pUserTag, this, sourceFile.toStdString().c_str() );
        m_pSelectionWindow->show();
    }
    catch(...){
        m_pSelectionWindow = nullptr;
        QMessageBox::warning(this,"Load Error","Input .ycl file is invalid or corrupt.");
        return;
    }

    /* Connect close signals in this & child dialog */
    QObject::connect(m_pSelectionWindow, &NodeSelectWindow::interfaceClose,
                        this, &MainWindow::ClearSelectionWindow);

    /*  Connect signals in this & child dialog */
    QObject::connect(m_pSelectionWindow, &NodeSelectWindow::addNodeToItem,
                        this, &MainWindow::AddNodeToTreeTag);

    /*  Connect signals in this & child dialog */
    QObject::connect(m_pSelectionWindow, &NodeSelectWindow::validateParent,
                        this, &MainWindow::ValidateLinks);
}


void
MainWindow::on_removenodebutton_clicked()
{
    QTreeWidgetItem* pTreeItem = ui->treeWidget->currentItem();
    DefWidgetItem* pDefItem = (DefWidgetItem*)pTreeItem;

    /* Evaluate parent index */
    if (!pDefItem) return;
    StTag* pSourceTag = pDefItem->getItemTag();
    int nodeIdx = GetParentChildNodeIndex(pSourceTag);

    /* Remove Element */
    if (nodeIdx == -1) return;
    removeElementAtIndex(pSourceTag->pParent->children,nodeIdx);
    delete pSourceTag; /* todo: delete all children */

    /* Reload Hierarchy */
    PopulateTreeWidget(m_clothObj->getRootTag());
    UpdateStatLabels();
}


void
MainWindow::dropEvent(QDropEvent *event)
{
    QString name;
    QList<QUrl> urls;
    QList<QUrl>::Iterator i;
    urls = event->mimeData()->urls();

    for(i = urls.begin(); i != urls.end(); ++i)
        name = i->toLocalFile();

    if (name != "" && name.endsWith(".ycl"))
    {
        OpenYukesClothFile(name);
    }

}

void
MainWindow::dragEnterEvent(QDragEnterEvent *event){
    event->acceptProposedAction();
}

void
MainWindow::AddContextMenuNodeSettings(QMenu& menu){
    QTreeWidgetItem* pTreeItem = ui->treeWidget->currentItem();
    DefWidgetItem* pDefItem = (DefWidgetItem*)pTreeItem;
    if (!pDefItem) return;

    StTag* pTag = pDefItem->getItemTag();
    if (!pTag->pSimMesh) return;

#ifdef DEBUG_EDITOR
    QAction* pExportJsonAct = ui->actionExportItemAsJson;
    pExportJsonAct->setText("Export \"" + pTreeItem->text(0) + "\" as json");
    menu.addAction(pExportJsonAct);

    QAction* pImportJsonAct = ui->actionImport_Custom_Constraints;
    menu.addAction(pImportJsonAct);
#endif


}
void
MainWindow::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu(this);
    menu.addAction(ui->actionAdd_Root_Node);
    menu.addAction(ui->actionRemove_Item);
    menu.addAction(ui->actionExpand_Collapse_All);
#ifdef DEBUG_EDITOR
    AddContextMenuNodeSettings(menu);
#endif
    menu.exec( ui->treeWidget->mapToGlobal(pos) );
}


void
MainWindow::on_actionAdd_Root_Node_triggered()
{
    on_addnodebutton_clicked();
}


void
MainWindow::on_actionRemove_Item_triggered()
{
    on_removenodebutton_clicked();
}


void
MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    on_treeWidget_itemDoubleClicked(item,column);
}


void
MainWindow::on_actionExpand_Collapse_All_triggered()
{
    on_actionExpand_Collapse_triggered();
}


void
MainWindow::on_actionExportItemAsJson_triggered()
{
    if (this->m_pSelectionWindow || !m_clothObj) return;
    QTreeWidgetItem* pTreeItem = ui->treeWidget->currentItem();
    DefWidgetItem* pDefItem = (DefWidgetItem*)pTreeItem;

    if (!pDefItem) return;
    StTag* pSourceTag = pDefItem->getItemTag();
    QString sourceFile = QFileDialog::getSaveFileName(this, tr("Save .json file"),
                                                         this->m_sExplorerPath,
                                                         tr("JSON (*.json)") );
    if (sourceFile == "" || !pSourceTag->pSimMesh){ return;}
    ClothJson json(pSourceTag,sourceFile.toStdString().c_str());
    json.save();

    QMessageBox::information(this,"File Saved", " Node/JSON operation complete.");
}

void
MainWindow::on_actionImport_Custom_Constraints_triggered()
{
    if (this->m_pSelectionWindow || !m_clothObj) return;
    QTreeWidgetItem* pTreeItem = ui->treeWidget->currentItem();
    DefWidgetItem* pDefItem = (DefWidgetItem*)pTreeItem;

    if (!pDefItem) return;
    StTag* pSourceTag = pDefItem->getItemTag();
    QString sourceFile = QFileDialog::getOpenFileName(this, tr("Open .json file"),
                                                         this->m_sExplorerPath,
                                                         tr("JSON (*.json)") );
    if (sourceFile == "" || !pSourceTag->pSimMesh) return;

    ClothJson json(pSourceTag,sourceFile.toStdString().c_str());
    json.UpdateTag();

    QMessageBox::information(this,"Modified Data", " JSON import complete.");
}

void
CreateSaveOverwritePrompt(QString path, bool* ok){
    QMessageBox msgBox;
    msgBox.setWindowTitle("Overwrite File");
    msgBox.setText("Are you sure you want to overwrite \"" + path+ "\"?");
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if(msgBox.exec() == QMessageBox::Yes){*ok = true;}
    else { *ok = false; }
}

void
MainWindow::on_actionSave_triggered()
{
    bool isSaveReady = false;
    CreateSaveOverwritePrompt(m_sYclFilePath,&isSaveReady);
    if (!isSaveReady)   return;
    if (m_clothObj->getRootTag()->children.size() == 0){
        QMessageBox::warning(this,"File Error", "YCL is missing a tag hierarchy.");
        return;}

    CClothSave::SaveToDisk(m_sYclFilePath.toStdString().c_str(), m_clothObj);
    QMessageBox::information(this,"Save Complete", "File saved to: " + m_sYclFilePath);
}



void
MainWindow::on_expandButton_clicked()
{
    on_actionExpand_Collapse_All_triggered();
}

QStringList
sortJsimFiles(QStringList files) {
    // Custom comparison function
    auto customCompare = [](const QString& file1, const QString& file2) {
        if (file1.endsWith("_source.jsim") && !file2.endsWith("_source.jsim")) {
            return false; // file1 comes after file2
        } else if (!file1.endsWith("_source.jsim") && file2.endsWith("_source.jsim")) {
            return true; // file1 comes before file2
        } else {
            return file1 < file2; // lexicographical comparison
        }
    };

    // Sort the files using custom comparison function
    std::sort(files.begin(), files.end(), customCompare);

    return files;
}

void
MainWindow::on_CreateButton_clicked()
{
    if (this->m_pSelectionWindow || !m_clothObj) return;
    QStringList sourceFiles = QFileDialog::getOpenFileNames(this, tr("Open .jsim file"),
                                                         this->m_sExplorerPath,
                                                         tr("JSIM (*.jsim)") );
    if (sourceFiles.size() == 0) return;
    bool importOk = false;
    QString filePath = m_sYclFilePath;
    sourceFiles = sortJsimFiles(sourceFiles);

    for (auto& file : sourceFiles){
        /* Load each jsim container and append to hierarchy */
        try{
            CJsonSimMesh jsonMesh(file.toStdString().c_str());
            StTag* importTag = jsonMesh.GetMeshTag();
            importOk = MainWindow::AddNodeToTreeTag(importTag);
        }
        catch(...){
            QMessageBox::warning(this,"File Error", "Could not load invalid or corrupt JSIM container.");
            return;
        }
    }

    if (importOk){
        /* Update file container with appdata cache */
        try { ReloadTempFile(); }
        catch (...)
        {
            ClearAllData();
            OpenYukesClothFile(filePath);
            QMessageBox::warning(this,"File Error", "Could not import mesh. Target .jsim is missing source sim file.");
            return;
        }

        QMessageBox::information(this,"File Load", "JSIM mesh import complete");
    }
}

void
MainWindow::ReloadTempFile(){
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (m_clothObj->getRootTag()->children.size() == 0)
        throw("Cannot save empty ycl file.");

    QDir dir(appDataPath);
    if (!dir.mkpath("."))
        throw("Failed to write contents.");

    QString tempFile = appDataPath + "/editor_temp.ycl";
    CClothSave::SaveToDisk(tempFile.toStdString().c_str(), m_clothObj);
    OpenAppDataYCLFile(tempFile);

    if (m_clothObj == nullptr)
        throw("Failed to write contents.");
}


void
MainWindow::ClearAllData(){
    m_sYclFilePath = "";
    if (m_clothObj){
        m_clothObj->CGameCloth::~CGameCloth();
        m_clothObj = nullptr;
    }

    if (m_pSelectionWindow){
        m_pSelectionWindow->close(); }
    m_pSelectionWindow = nullptr;

    ui->treeWidget->clear();
    ui->tableWidget->clear();
    ui->nodeLabel->setText("0 node(s)");
    ui->pathLabel->setText( "YCL Editor" );
    ui->meshTotalLabel->setText("SimMesh Total: 0");
    ui->lineTotalLabel->setText("SimLine Total: 0");
    ui->colTotalLabel->setText("Collision Total: 0");
}


void
MainWindow::on_actionClear_File_triggered()
{
    ClearAllData();
}

bool HasSimLinkTarget(StSimMesh* pMesh, const StTag* pTag)
{
   for (auto& child : pTag->children){
       StSimMesh* pTreeMesh = child->pSimMesh;
       if(pTreeMesh)
       {
           if (pMesh->target.source == pTreeMesh->sObjName) return true;
           if (pMesh->sObjName == pTreeMesh->sObjName) return false;
       }
   }

   return false;
}

void
MainWindow::ValidateLinks()
{
   if (!m_clothObj) return;
   const StTag* pTagHead = m_clothObj->getRootTag();

   for (int i = 0; i < pTagHead->children.size(); i++)
   {
        auto child = pTagHead->children.at(i);
        StSimMesh* mesh = child->pSimMesh;

        if(mesh && !mesh->target.indices.empty() && !HasSimLinkTarget(mesh, pTagHead))
        {
           QString filePath = m_sYclFilePath;
           this->ClearAllData();
           OpenYukesClothFile(filePath);
           QString log = "Could not load node. Target mesh: \"" + QString::fromStdString(mesh->sObjName)
                         + "\" is missing sim mesh: \"" + QString::fromStdString(mesh->target.source) + "\"";
           QMessageBox::warning(this,"File Error",log);
           return;
        }
   }
}


void MainWindow::on_actionRemoveTreeItem_triggered()
{
    MainWindow::on_actionRemove_Item_triggered();
}





