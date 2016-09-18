#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QApplication>


MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent),  m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    m_gl=new Draw(this);
    qString = new QTextStream();

    m_ui->s_mainWindowGridLayout->addWidget(m_gl,0,0,12,1);

    connect(m_ui->load_xml_button, SIGNAL(clicked()), this, SLOT(open_folder()));
    connect(m_ui->import_particles_button, SIGNAL(clicked()), this, SLOT(import_particle()));

    connect(m_ui->start_sim_button, SIGNAL(clicked()), this, SLOT(start_simulation()));
    connect(m_ui->export_sim_button, SIGNAL(clicked()), this, SLOT(export_simulation()));

    connect(m_ui->reset_button, SIGNAL(clicked()), this, SLOT(reset()));
}

MainWindow::~MainWindow()
{
    delete m_ui;
    delete m_gl;
    delete qString;
}

void MainWindow::init()
{

}

// ------------------------ open folder -------------------------
void MainWindow::open_folder()
{
  this->hide();
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open XML File"), "/home", tr("XML Files (*.xml)"));
  //QString fileName = "/home/i7634158/PBDFluidXML.xml";
  std::cout<<fileName.toStdString()<<std::endl;

  if(fileName.isEmpty())
  {
    QMessageBox::critical(this, "", "Please select a xml file.", QMessageBox::Ok);
    std::cout<<"xml file missing..."<<std::endl;
  }

  this->show();

  if( load_xml(fileName) )
  {
    std::cout<<"xml load done."<<std::endl;
    m_gl->m_sim_ptr->initParameter();

    m_ui->load_xml_button->setEnabled(false);
  }
}

// ------------------------ load xml -------------------------
bool MainWindow::load_xml(QString _filePath)
{
  QString temp_str = "";
  QString data = "";
  xmlFile = new QFile(_filePath);

  m_ui->xml_path->setText(_filePath.append("\n"));

  if(!xmlFile->open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QMessageBox::critical(this, "", "Fail to load xml.", QMessageBox::Ok);
    std::cout<<"Fail to load xml."<<std::endl;
    return false;
  }

  xmlReader = new QXmlStreamReader(xmlFile);
  while( !xmlReader->atEnd() && !xmlReader->hasError() )
  {
    QXmlStreamReader::TokenType token = xmlReader->readNext();
    //If token is just StartDocument - go to next
    if(token == QXmlStreamReader::StartDocument)
    {
            continue;
    }
    //If token is StartElement - read it
    if(token == QXmlStreamReader::StartElement)
    {
      if(xmlReader->name() == "pbf_xml")
      {
        continue;
      }

      if(xmlReader->name() == "m_tank_size")
      {
        data = xmlReader->readElementText();
        temp_str.append("Tank Size=").append(data).append(" \n");
        display_data(temp_str);
        Definition::getInstance()->m_tank_size = data.toUInt();
        std::cout<<"m_tank_size="<<Definition::getInstance()->m_tank_size<<std::endl;
      }
      if(xmlReader->name() == "m_kernel_h")
      {
        data = xmlReader->readElementText();
        temp_str.append("Kernel H=").append(data).append(" \n");
        display_data(temp_str);
        Definition::getInstance()->m_kernel_h = data.toFloat();
        Definition::getInstance()->m_kernel_2h = data.toFloat()*data.toFloat();
        std::cout<<"kernel="<<Definition::getInstance()->m_kernel_h<<std::endl;
      }
      if(xmlReader->name() == "m_rest_density")
      {
        data = xmlReader->readElementText();
        temp_str.append("Rest Density=").append(data).append(" \n");
        display_data(temp_str);
        Definition::getInstance()->m_rest_density = data.toFloat();
        std::cout<<"m_rest_density="<<Definition::getInstance()->m_rest_density<<std::endl;
      }
      if(xmlReader->name() == "m_particle_raduis")
      {
        data = xmlReader->readElementText();
        temp_str.append("Particle Raduis=").append(data).append(" \n");
        display_data(temp_str);
        Definition::getInstance()->m_particle_raduis = data.toFloat();
        std::cout<<"m_particle_raduis="<<Definition::getInstance()->m_particle_raduis<<std::endl;
      }
      if(xmlReader->name() == "m_particle_mass")
      {
        data = xmlReader->readElementText();
        temp_str.append("Particle Mass=").append(data).append(" \n");
        display_data(temp_str);
        Definition::getInstance()->m_particle_mass = data.toFloat();
        std::cout<<"m_particle_mass="<<Definition::getInstance()->m_particle_mass<<std::endl;
      }
      if(xmlReader->name() == "m_gravity")
      {
        data = xmlReader->readElementText();
        temp_str.append("Gravity=").append(data).append(" \n");
        display_data(temp_str);
        Definition::getInstance()->m_gravity = data.toFloat();
        std::cout<<"m_gravity="<<Definition::getInstance()->m_gravity<<std::endl;
      }
      if(xmlReader->name() == "m_max_iteration")
      {
        data = xmlReader->readElementText();
        temp_str.append("Max Iteration=").append(data).append(" \n");
        display_data(temp_str);
        Definition::getInstance()->m_max_iteration = data.toFloat();
        std::cout<<"m_max_iteration="<<Definition::getInstance()->m_max_iteration<<std::endl;
      }
      if(xmlReader->name() == "m_time_step")
      {
        data = xmlReader->readElementText();
        temp_str.append("Time Step=").append(data).append(" \n");
        display_data(temp_str);
        Definition::getInstance()->m_time_step = data.toFloat();
        std::cout<<"m_time_step="<<Definition::getInstance()->m_time_step<<std::endl;
      }
      if(xmlReader->name() == "m_epo")
      {
        data = xmlReader->readElementText();
        temp_str.append("EPO=").append(data).append(" \n");
        display_data(temp_str);
        Definition::getInstance()->m_epo = data.toFloat();
        std::cout<<"m_epo="<<Definition::getInstance()->m_epo<<std::endl;
      }
    }
  }

  if(xmlReader->hasError())
  {
    QMessageBox::critical(this, "", "Xml file Parse Error", QMessageBox::Ok);
    return false;
  }

  delete xmlReader;
  delete xmlFile;
  return true;
}

// ----------------------- show data in message box ----------------------
void MainWindow::display_data(QString _qstring)
{
  m_ui->textBrowser->setText(_qstring);
}

// -------------------------------- Load .obj -------------------------------------
void MainWindow::import_particle()
{
  this->hide();
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Obj File"), "/home", tr("Obj Files (*.obj)"));
  //QString fileName = "/home/i7634158/dambreak_2_2.obj";
  std::cout<<fileName.toStdString()<<std::endl;

  if(fileName.isEmpty())
  {
    QMessageBox::critical(this, "", "Please select an obj file.", QMessageBox::Ok);
    std::cout<<"obj file missing..."<<std::endl;
  }

  FILE *file = fopen(fileName.toStdString().c_str(), "r");
  if( file == NULL )
  {
      std::cout<<"Impossible to open the file!"<<std::endl;
  }

  this->show();

  QString temp_str;
  temp_str.append(fileName);
  m_ui->obj_path->setText(temp_str);

  if( loadOBJ(file, fileName.toStdString().c_str()) )
  {
    m_gl->m_sim_ptr->initParticleObj(m_temp_vertices);
    m_ui->import_particles_button->setEnabled(false);
    std::cout<<"obj load done."<<std::endl;
  }
}

bool MainWindow::loadOBJ(FILE *file, const char * path)
{
  m_temp_vertices.clear();

  while(1)
  {
    char lineHeader[128];
    // read the first word of the line
    int res = fscanf(file, "%s", lineHeader);
    if (res == EOF)
      break;

    if ( strcmp( lineHeader, "v" ) == 0 )
    {
      ngl::Vec3 vertex;
      fscanf(file, "%f %f %f\n", &vertex.m_x, &vertex.m_y, &vertex.m_z );
      m_temp_vertices.push_back(vertex);
    }
    else if ( strcmp( lineHeader, "vt" ) == 0 )
    {
      ngl::Vec2 uv;
      fscanf(file, "%f %f\n", &uv.m_x, &uv.m_y );
      //m_temp_uvs.push_back(uv);
    }
    else if ( strcmp( lineHeader, "vn" ) == 0 )
    {
      ngl::Vec3 normal;
      fscanf(file, "%f %f %f\n", &normal.m_x, &normal.m_y, &normal.m_z );
      //m_temp_normals.push_back(normal);
    }
    /*else if ( strcmp( lineHeader, "f" ) == 0 )
    {
      std::string vertex1, vertex2, vertex3;
      unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
      int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
      if (matches != 9)
      {
          printf("File can't be read by our simple parser : ( Try exporting with other options\n");
          return false;
      }
      vertexIndices.push_back(vertexIndex[0]);
      vertexIndices.push_back(vertexIndex[1]);
      vertexIndices.push_back(vertexIndex[2]);
      uvIndices    .push_back(uvIndex[0]);
      uvIndices    .push_back(uvIndex[1]);
      uvIndices    .push_back(uvIndex[2]);
      normalIndices.push_back(normalIndex[0]);
      normalIndices.push_back(normalIndex[1]);
      normalIndices.push_back(normalIndex[2]);
    }*/
  }

  return true;
}

void MainWindow::start_simulation()
{
  m_gl->m_sim_ptr->toggleSimulation();
}

void MainWindow::export_simulation()
{
  m_gl->m_sim_ptr->toggleExport();
}

void MainWindow::reset()
{
  // GUI
  m_ui->load_xml_button->setEnabled(true);
  m_ui->import_particles_button->setEnabled(true);
  m_ui->xml_path->clear();
  m_ui->obj_path->clear();
  m_ui->textBrowser->clear();
  m_msg.clear();

  // Sim Data
  m_gl->m_sim_ptr->reset();



}
