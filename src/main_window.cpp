#include "main_window.h"

#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QStyleOptionComboBox>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QWidget>

#include "drop_image_label.h"
#include "image_io.h"
#include "image_qt_converter.h"
#include "parser.h"
#include "pipeline_builder.h"

namespace {

class CenteredItemDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignCenter;
    }
};

class FilterComboBox : public QComboBox {
public:
    using QComboBox::QComboBox;

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);

        QStyleOptionComboBox option;
        initStyleOption(&option);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRect rect = this->rect();
        painter.setPen(QColor("#374151"));
        painter.setBrush(QColor("#111827"));
        painter.drawRoundedRect(rect.adjusted(0, 0, -1, -1), 10, 10);

        const QRect arrow_zone(rect.right() - 34, rect.top(), 34, rect.height());

        QRect text_rect = rect.adjusted(12, 0, -40, 0);
        painter.setPen(QColor("#f9fafb"));
        painter.drawText(text_rect, Qt::AlignCenter, currentText());

        QPoint p1(arrow_zone.center().x() - 6, arrow_zone.center().y() - 3);
        QPoint p2(arrow_zone.center().x() + 6, arrow_zone.center().y() - 3);
        QPoint p3(arrow_zone.center().x(), arrow_zone.center().y() + 5);

        QPolygon arrow;
        arrow << p1 << p2 << p3;

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#f9fafb"));
        painter.drawPolygon(arrow);
    }
};

}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    setStyleSheet(
        "QMainWindow {"
        "  background-color: #0f172a;"
        "}"
        "QWidget {"
        "  background-color: #0f172a;"
        "  color: #e5e7eb;"
        "}"
        "QPushButton {"
        "  background-color: #1f2937;"
        "  color: #f9fafb;"
        "  border: 1px solid #374151;"
        "  border-radius: 10px;"
        "  padding: 8px 12px;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #374151;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #4b5563;"
        "}"
        "QLineEdit {"
        "  background-color: #111827;"
        "  color: #f9fafb;"
        "  border: 1px solid #374151;"
        "  border-radius: 10px;"
        "  padding: 6px 10px;"
        "}"
        "QLabel {"
        "  color: #e5e7eb;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: #111827;"
        "  color: #f9fafb;"
        "  border: 1px solid #374151;"
        "  selection-background-color: #2563eb;"
        "  selection-color: #ffffff;"
        "}"
    );

    open_button_ = new QPushButton("Open image");
    apply_button_ = new QPushButton("Apply filter");
    save_button_ = new QPushButton("Save image");

    open_button_->setMinimumHeight(38);
    apply_button_->setMinimumHeight(38);
    save_button_->setMinimumHeight(38);

    filter_box_ = new FilterComboBox();
    filter_box_->setView(new QListView(this));
    filter_box_->setItemDelegate(new CenteredItemDelegate(filter_box_));
    filter_box_->setMinimumHeight(36);
    filter_box_->setCursor(Qt::PointingHandCursor);
    filter_box_->setStyleSheet(
        "QComboBox {"
        "  background-color: #111827;"
        "  color: #f9fafb;"
        "  border: 1px solid #374151;"
        "  border-radius: 10px;"
        "  padding: 6px 12px;"
        "  min-height: 36px;"
        "}"
        "QComboBox::drop-down {"
        "  width: 34px;"
        "  border: none;"
        "}"
    );

    filter_box_->addItem("Grayscale");
    filter_box_->addItem("Negative");
    filter_box_->addItem("Sharpen");
    filter_box_->addItem("Crop");
    filter_box_->addItem("Edge detection");
    filter_box_->addItem("Gaussian blur");
    filter_box_->addItem("Glow");
    filter_box_->addItem("Contrast");

    image_label_ = new DropImageLabel();

    param1_label_ = new QLabel();
    param2_label_ = new QLabel();
    param3_label_ = new QLabel();

    param1_edit_ = new QLineEdit();
    param2_edit_ = new QLineEdit();
    param3_edit_ = new QLineEdit();

    param1_label_->setStyleSheet("color: #cbd5e1; font-size: 13px;");
    param2_label_->setStyleSheet("color: #cbd5e1; font-size: 13px;");
    param3_label_->setStyleSheet("color: #cbd5e1; font-size: 13px;");

    param1_label_->setAlignment(Qt::AlignCenter);
    param2_label_->setAlignment(Qt::AlignCenter);
    param3_label_->setAlignment(Qt::AlignCenter);

    param1_edit_->setMinimumHeight(34);
    param2_edit_->setMinimumHeight(34);
    param3_edit_->setMinimumHeight(34);

    param1_edit_->setPlaceholderText("Enter value");
    param2_edit_->setPlaceholderText("Enter value");
    param3_edit_->setPlaceholderText("Enter value");

    auto* controls_layout = new QVBoxLayout();
    controls_layout->setSpacing(12);
    controls_layout->addWidget(open_button_);
    controls_layout->addWidget(filter_box_);
    controls_layout->addWidget(param1_label_);
    controls_layout->addWidget(param1_edit_);
    controls_layout->addWidget(param2_label_);
    controls_layout->addWidget(param2_edit_);
    controls_layout->addWidget(param3_label_);
    controls_layout->addWidget(param3_edit_);
    controls_layout->addWidget(apply_button_);
    controls_layout->addWidget(save_button_);
    controls_layout->addStretch();

    auto* controls_widget = new QWidget();
    controls_widget->setLayout(controls_layout);
    controls_widget->setFixedWidth(260);

    auto* main_layout = new QHBoxLayout();
    main_layout->setSpacing(20);
    main_layout->setContentsMargins(20, 20, 20, 20);
    main_layout->addWidget(controls_widget);
    main_layout->addWidget(image_label_, 1);

    central_widget->setLayout(main_layout);

    connect(open_button_, &QPushButton::clicked, this, &MainWindow::OpenImage);
    connect(apply_button_, &QPushButton::clicked, this, &MainWindow::ApplyFilter);
    connect(save_button_, &QPushButton::clicked, this, &MainWindow::SaveImage);
    connect(filter_box_, &QComboBox::currentIndexChanged, this, &MainWindow::OnFilterChanged);
    connect(image_label_, &DropImageLabel::FileDropped, this, &MainWindow::LoadImageFromPath);

    setWindowTitle("Image Processor");
    resize(1150, 720);

    UpdateParameterFields();
}

void MainWindow::OpenImage() {
    const QString file_name = QFileDialog::getOpenFileName(
        this,
        "Open image",
        "",
        "Images (*.bmp *.png *.jpg *.jpeg *.heic *.heif)"
    );

    if (file_name.isEmpty()) {
        return;
    }

    LoadImageFromPath(file_name);
}

void MainWindow::LoadImageFromPath(const QString& path) {
    try {
        current_image_ = ReadImage(path.toStdString());
        processed_image_ = current_image_;
        has_image_ = true;
        has_processed_image_ = true;
        UpdatePreview();
    } catch (const std::exception& error) {
        QMessageBox::critical(this, "Error", error.what());
    }
}

void MainWindow::ApplyFilter() {
    if (!has_image_) {
        QMessageBox::warning(this, "Warning", "Load an image first");
        return;
    }

    try {
        processed_image_ = current_image_;

        std::vector<FilterSpec> filters;
        const QString filter_name = filter_box_->currentText();

        if (filter_name == "Grayscale") {
            filters.push_back({"gs", {}});
        } else if (filter_name == "Negative") {
            filters.push_back({"neg", {}});
        } else if (filter_name == "Sharpen") {
            filters.push_back({"sharp", {}});
        } else if (filter_name == "Crop") {
            filters.push_back({"crop", {
                param1_edit_->text().toStdString(),
                param2_edit_->text().toStdString()
            }});
        } else if (filter_name == "Edge detection") {
            filters.push_back({"edge", {
                param1_edit_->text().toStdString()
            }});
        } else if (filter_name == "Gaussian blur") {
            filters.push_back({"blur", {
                param1_edit_->text().toStdString()
            }});
        } else if (filter_name == "Glow") {
            filters.push_back({"glow", {
                param1_edit_->text().toStdString(),
                param2_edit_->text().toStdString(),
                param3_edit_->text().toStdString()
            }});
        } else if (filter_name == "Contrast") {
            filters.push_back({"contrast", {
                param1_edit_->text().toStdString()
            }});
        }

        const Pipeline pipeline = PipelineBuilder::Build(filters);
        pipeline.Apply(processed_image_);

        has_processed_image_ = true;
        UpdatePreview();
    } catch (const std::exception& error) {
        QMessageBox::critical(this, "Error", error.what());
    }
}

void MainWindow::SaveImage() {
    if (!has_processed_image_) {
        QMessageBox::warning(this, "Warning", "Nothing to save");
        return;
    }

    const QString file_name = QFileDialog::getSaveFileName(
        this,
        "Save image",
        "",
        "Images (*.bmp *.png *.jpg *.jpeg *.heic *.heif)"
    );

    if (file_name.isEmpty()) {
        return;
    }

    try {
        WriteImage(file_name.toStdString(), processed_image_);
    } catch (const std::exception& error) {
        QMessageBox::critical(this, "Error", error.what());
    }
}

void MainWindow::OnFilterChanged() {
    UpdateParameterFields();
}

void MainWindow::UpdateParameterFields() {
    const QString filter_name = filter_box_->currentText();

    param1_label_->hide();
    param2_label_->hide();
    param3_label_->hide();

    param1_edit_->hide();
    param2_edit_->hide();
    param3_edit_->hide();

    param1_edit_->clear();
    param2_edit_->clear();
    param3_edit_->clear();

    if (filter_name == "Crop") {
        param1_label_->setText("Width");
        param2_label_->setText("Height");

        param1_label_->show();
        param2_label_->show();
        param1_edit_->show();
        param2_edit_->show();
    } else if (filter_name == "Edge detection") {
        param1_label_->setText("Threshold");

        param1_label_->show();
        param1_edit_->show();
    } else if (filter_name == "Gaussian blur") {
        param1_label_->setText("Sigma");

        param1_label_->show();
        param1_edit_->show();
    } else if (filter_name == "Glow") {
        param1_label_->setText("Threshold");
        param2_label_->setText("Radius");
        param3_label_->setText("Intensity");

        param1_label_->show();
        param2_label_->show();
        param3_label_->show();
        param1_edit_->show();
        param2_edit_->show();
        param3_edit_->show();
    } else if (filter_name == "Contrast") {
        param1_label_->setText("Coefficient");

        param1_label_->show();
        param1_edit_->show();
    }
}

void MainWindow::UpdatePreview() {
    if (!has_processed_image_) {
        return;
    }

    const QImage qimage = ToQImage(processed_image_);
    image_label_->setStyleSheet(
        "QLabel {"
        "  border: 1px solid #374151;"
        "  border-radius: 18px;"
        "  background-color: #111827;"
        "}"
    );
    image_label_->setText("");
    image_label_->setPixmap(
        QPixmap::fromImage(qimage).scaled(
            image_label_->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        )
    );
}
