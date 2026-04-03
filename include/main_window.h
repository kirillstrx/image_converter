#pragma once

#include <QMainWindow>

#include "image.h"

class QPushButton;
class QComboBox;
class QLabel;
class QLineEdit;
class DropImageLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void OpenImage();
    void ApplyFilter();
    void SaveImage();
    void OnFilterChanged();
    void LoadImageFromPath(const QString& path);

private:
    void UpdatePreview();
    void UpdateParameterFields();

    DropImageLabel* image_label_;
    QPushButton* open_button_;
    QPushButton* apply_button_;
    QPushButton* save_button_;
    QComboBox* filter_box_;

    QLabel* param1_label_;
    QLabel* param2_label_;
    QLabel* param3_label_;

    QLineEdit* param1_edit_;
    QLineEdit* param2_edit_;
    QLineEdit* param3_edit_;

    Image current_image_;
    Image processed_image_;

    bool has_image_ = false;
    bool has_processed_image_ = false;
};