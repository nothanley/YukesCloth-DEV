#include <QMainWindow>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <YukesCloth>
#pragma once

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTableWidget;
class DefWidgetItem;
class QTreeWidgetItem;
class CClothContainer;
class NodeSelectWindow;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void OpenYukesClothFile(const QString& filePath);

private slots:
    void on_OpenFile_clicked();
    void on_actionOpen_ycl_file_triggered();
    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_actionExpand_Collapse_triggered();
    void on_actionExit_triggered();
    void on_SaveButton_clicked();
    void on_actionSave_File_triggered();
    void on_addnodebutton_clicked();
    void on_removenodebutton_clicked();
    void on_treeWidget_customContextMenuRequested(const QPoint &pos);
    void on_actionAdd_Root_Node_triggered();
    void on_actionRemove_Item_triggered();
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_actionExpand_Collapse_All_triggered();
    void ClearSelectionWindow();
    void AddNodeToTreeTag(StTag* newTag);
    void on_actionExportItemAsJson_triggered();
    void on_actionImport_Custom_Constraints_triggered();
    void on_actionSave_triggered();

    void on_expandButton_clicked();

    void on_CreateButton_clicked();

private:
    Ui::MainWindow *ui;
    void appendNodeToView(StTag *pSourceTag, DefWidgetItem* pParentItem);
    void UpdateStatLabels();
    void PopulateTreeWidget(StTag* pRootNode);
    void PopulateTableWidget(StTag* pNode, QTableWidget *pTable);
    void AddContextMenuNodeSettings(QMenu &menu);

private:
    QString m_sYclFilePath;
    CClothContainer* m_pYclFile = nullptr;
    NodeSelectWindow* m_pSelectionWindow = nullptr;
    QString m_sExplorerPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation); // Cache Explorer Directory
    bool isExpanded = false;

#ifdef DEBUG_EDITOR
    void InitializeDebugWidgets();
    QTextEdit* m_DbgConsoleLog = nullptr;
    QTextEdit* m_DbgTextWidget = nullptr;
#endif

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

};
