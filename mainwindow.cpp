#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QXmlStreamWriter>

class XmlEnumerator
{
public:
    XmlEnumerator() : writer(&buffer) {
        writer.setAutoFormatting(true);
        // The indentation is by 1 in the UI.
        writer.setAutoFormattingIndent(1);
    }

    QString xmlOutput() { return buffer; }

    void enumerateMenu(QMenu *menu, int level)
    {
        // This is how the XML looks:
        //
        // <widget class="QMenu" name="menuFoo">
        //  <property name="title">
        //   <string>Foo</string>
        //  </property>
        //  ................
        // </widget>

        // First we write the header of the menu.
        writer.writeStartElement("widget");
        writer.writeAttribute("class", "QMenu");
        writer.writeAttribute("name", menu->objectName());
        writer.writeStartElement("property");
        writer.writeAttribute("name", "title");
        writer.writeTextElement("string", menu->title());
        writer.writeEndElement();

        // Now write all the nested actions and submenus.
        foreach (QObject *ii, menu->findChildren<QObject*>()) {
            if (QMenu *menu = qobject_cast<QMenu*>(ii)) {
                enumerateMenu(menu, level+1);
            } else if (QAction *action = qobject_cast<QAction *>(ii)) {
                // Actions are just referenced here. Their full description is
                // a top-level xml entry. Top-level means it's a child of the
                // QMainWindow tag itself. They are outside the scope of the
                // menu bar widget xml tag.

                // <addaction name="actionBar1"/>
                writer.writeTextElement("addaction", action->objectName());
            }
        }

        // Close the widget XML tag.
        writer.writeEndElement();
    }

    // I use a separate function so it can receive a QMenuBar pointer.
    // Otherwise it's the same code as enumerateMenu().
    void enumerateMenuBar(QMenuBar *menuBar)
    {
        foreach (QMenu *menu, menuBar->findChildren<QMenu*>()) {
            enumerateMenu(menu, 1);
        }
    }

private:
    QString buffer;
    QXmlStreamWriter writer;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    XmlEnumerator enumerator;
    enumerator.enumerateMenuBar(ui->menuBar);
    qDebug().noquote() << enumerator.xmlOutput();
}

MainWindow::~MainWindow()
{
    delete ui;
}
