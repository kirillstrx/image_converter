#include "drop_image_label.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>

namespace {

    const char* kIdleStyle =
        "QLabel {"
        "  border: 2px dashed #4b5563;"
        "  border-radius: 18px;"
        "  background-color: #111827;"
        "  color: #d1d5db;"
        "  font-size: 18px;"
        "  font-weight: 500;"
        "  padding: 24px;"
        "}";

    const char* kActiveStyle =
        "QLabel {"
        "  border: 2px dashed #60a5fa;"
        "  border-radius: 18px;"
        "  background-color: #1f2937;"
        "  color: #93c5fd;"
        "  font-size: 18px;"
        "  font-weight: 600;"
        "  padding: 24px;"
        "}";

}  // namespace

DropImageLabel::DropImageLabel(QWidget* parent) : QLabel(parent) {
    setAcceptDrops(true);
    setAlignment(Qt::AlignCenter);
    setText("Drag and drop an image here\nor use Open image");
    setStyleSheet(kIdleStyle);
}

void DropImageLabel::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile()) {
            event->acceptProposedAction();
            setStyleSheet(kActiveStyle);
            return;
        }
    }
    event->ignore();
}

void DropImageLabel::dropEvent(QDropEvent* event) {
    setStyleSheet(kIdleStyle);

    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) {
        event->ignore();
        return;
    }

    const QString path = urls.first().toLocalFile();
    if (path.isEmpty()) {
        event->ignore();
        return;
    }

    emit FileDropped(path);
    event->acceptProposedAction();
}