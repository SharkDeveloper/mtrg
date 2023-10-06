#include "usersdialog.h"
#include "ui_usersdialog.h"

#include <QMenu>
#include "device/settings.h"

UsersDialog::UsersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UsersDialog)
{
    ui->setupUi(this);

    for (int i = 0; i < Settings::instance().controllers.size(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(Settings::instance().controllers.at(i), ui->listWidget);
        item->setFlags (item->flags () | Qt::ItemIsEditable);
        ui->listWidget->addItem(item);
    }

    connect(ui->addButton, &QPushButton::pressed, this, [&]() {
        QListWidgetItem *item = new QListWidgetItem("Новый контроллер", ui->listWidget);
        item->setFlags (item->flags () | Qt::ItemIsEditable);
        ui->listWidget->addItem(item);
    });

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, [&](QPoint p) {
        if (ui->listWidget->itemAt(p))
        {
            QPoint globalPos = ui->listWidget->mapToGlobal(p);

            // Create menu and insert some actions
            QMenu menu;
            menu.addAction("Удалить", this, [&]() {
                // If multiple selection is on, we need to erase all selected items
                  for (int i = 0; i < ui->listWidget->selectedItems().size(); ++i) {
                      // Get curent item on selected row
                      QListWidgetItem *item = ui->listWidget->takeItem(ui->listWidget->currentRow());
                      // And remove it
                      delete item;
                  }
            });
            // Show context menu at handling position
            menu.exec(globalPos);
        }
    });

    connect(this, &QDialog::accepted, this, [=]() {
        Settings::instance().controllers.clear();
       for (int i = 0; i < ui->listWidget->count(); i++)
       {
            Settings::instance().controllers << ui->listWidget->item(i)->text();
       }

    });

}

UsersDialog::~UsersDialog()
{
    delete ui;
}
