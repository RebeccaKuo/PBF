/*
 * from 1. QuatSlerp ngl example
 * from 2. http://www.opengl-tutorial.org/ (Tutorial 7 : Model loading)
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QtXml/QDomDocument>
#include <QTextStream>
#include <QDebug>
#include "Draw.h"

namespace Ui {
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void init();
    Draw *m_gl;

private:
    Ui::MainWindow *m_ui;
    QString m_msg;

    //--------------------- load obj --------------------
    std::vector<ngl::Vec3> m_temp_vertices;

    bool loadOBJ(FILE *file, const char * path);

    //--------------------- read xml ----------------------
    QFile *xmlFile;
    QXmlStreamReader *xmlReader;
    QTextStream *qString;

private slots :

    void open_folder();
    bool load_xml(QString _filePath);
    void display_data(QString _qstring);
    void import_particle();
    void start_simulation();
    void export_simulation();
    void reset();
};

#endif
