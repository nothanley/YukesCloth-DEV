#include <QTreeWidget>
#include <QPainter>
#include <QDebug>
#include <QStyledItemDelegate>
#include <QTreeWidgetItem>
#include <QApplication>
#pragma once

class TreeLabelDelegate : public QStyledItemDelegate
{
public:
    TreeLabelDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void specifyGraphic(const uint32_t& tagType, QString& labelText, QPainter* pPainter) const
    {
        pPainter->setFont(QFont("Segoe UI",7,70));
        labelText = " " + labelText;

        switch(tagType)
        {
            case 0x5:
                pPainter->setBrush(QColor(41, 69, 184));
                labelText = " MESH";
                break;
            case 0x1B:
                pPainter->setBrush(QColor(2, 176, 103));
                labelText = " LINE";
                break;
            case 0x1:
                pPainter->setBrush(QColor(170, 55, 125));
                labelText = " COLS";
                break;
            case 0x19:
                pPainter->setBrush(QColor(163, 93, 62));
                labelText = " NODE";
                break;
            default:
                if (labelText.size() < 5)
                    labelText =  " " + labelText;
                break;
        }

    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem opt = option;
        this->initStyleOption(&opt, index);

        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        painter->setBrush(QColor(214, 46, 255,0));
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

        int squareSize = 34; // Adjust this size as needed
        int margin = 12;     // Adjust the margin as needed
        QRect itemRect = option.rect;
        QRect squareRect = itemRect.adjusted(margin, margin*1.25, -itemRect.width() + squareSize + margin, -margin*1.25);

        // Set the painter to draw the red square
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(60, 60, 60));

        // Access the custom attribute from the QTreeWidgetItem
        QString itemText = index.data().toString();
        uint32_t tagType = index.data(Qt::UserRole).toUInt();
        QString labelText = "0x" + QString::number(tagType,16).toUpper();
        specifyGraphic(tagType,labelText,painter);

        int cornerRadius = 1; // Adjust the corner radius as needed
        painter->drawRoundedRect(squareRect, cornerRadius, cornerRadius);

        // Set the painter to draw the Tree text
        painter->setFont(QFont("Segoe UI",9,55));
        painter->setPen( QColor(190, 190, 190) ); // You can change the text color as needed
        QRect textRect = itemRect.adjusted(squareSize + 1.75 * margin, 0, 0, 0); // Adjust the padding as needed
        painter->drawText(textRect, Qt::AlignVCenter, itemText );

        // Set the painter to draw the Square text
        painter->setPen( QColor(210, 210, 210) );
        QRect labelRect = itemRect.adjusted( squareSize - 20, -2, 0, 0); // Adjust the padding as needed
        painter->setFont(QFont("Segoe UI",8,70));

        specifyGraphic(tagType,labelText,painter);
        painter->drawText(labelRect, Qt::AlignVCenter, labelText );

        // Draw the item's text after the square
        QRect textClipRect = option.rect.adjusted(squareSize + 2 * margin, 0, 0, 0); // Clip the text to avoid overlapping
        painter->setClipRect(textClipRect);
        painter->restore();
    }

    void setTreeWidget(QTreeWidget* treePtr){ this->m_TreeWidget = treePtr; }

private:
    QTreeWidget* m_TreeWidget = nullptr;
};
