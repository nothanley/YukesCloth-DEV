#include <QDialog>
#include <YukesCloth>
#include <QListWidgetItem>
#pragma once

//class CSimObj;
//class StTag;

namespace Ui {
    class NodeSelectWindow;
}

class NodeSelectWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NodeSelectWindow(CSimObj* pSimObject, StTag* pTargetTag, QWidget *parent = nullptr, const char* fileName = "");
    ~NodeSelectWindow();

Q_SIGNALS:
    void interfaceClose();
    void addNodeToItem(StTag* newTag);
    void validateParent();

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_listWidget_itemSelectionChanged();

private:
    void SetupListWidget();
    void UpdateGUILabels();

private:
    std::vector<QListWidgetItem*> SortYCLNodes(const std::vector<QListWidgetItem*>& items);
    std::vector<QListWidgetItem*> GetAllSelectedMeshItems();

    Ui::NodeSelectWindow *ui;
    CSimObj* pSimObj = nullptr;
    StTag* m_pParentTag = nullptr;
    std::vector<StTag*> m_tagPalette;
    std::string m_sFileName;
};

