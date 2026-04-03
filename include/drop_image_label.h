#pragma once

#include <QLabel>

class DropImageLabel : public QLabel {
    Q_OBJECT

public:
    explicit DropImageLabel(QWidget* parent = nullptr);

    signals:
        void FileDropped(const QString& path);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
};