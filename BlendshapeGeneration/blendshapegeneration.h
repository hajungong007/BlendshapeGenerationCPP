#ifndef BLENDSHAPEGENERATION_H
#define BLENDSHAPEGENERATION_H

#include <QtWidgets/QMainWindow>
#include "ui_blendshapegeneration.h"

#include "OpenGL/gl3dcanvas.h"
#include <MultilinearReconstruction/basicmesh.h>

class OffscreenBlendshapeVisualizer {
public:
  OffscreenBlendshapeVisualizer(int w, int h);
  ~OffscreenBlendshapeVisualizer();

  void loadMesh(const string& filename);
  void loadReferenceMesh(const string& filename);

  int width() const { return canvasW; }
  int height() const { return canvasH; }

  QImage render() {
    paint();
    return rendered_img;
  }

  QImage getImage() {
    return rendered_img;
  }

  void repaint() {

  }

protected:
  void setupViewing();
  void paint();
  void enableLighting();
  void disableLighting();
  void computeDistance();
  void drawMesh(const BasicMesh& m);
  void drawMeshWithColor(const BasicMesh& m);
  void drawMeshVerticesWithColor(const BasicMesh& m);
  void drawColorBar(double, double);

private:
  vector<double> dists;
  BasicMesh mesh;
  BasicMesh refmesh;

  double sceneScale;
  QVector3D cameraPos;

  int canvasW, canvasH;
  enum ProjectionMode {
    ORTHONGONAL,
    FRUSTUM,
    PERSPECTIVE
  } projectionMode;
  QImage rendered_img;
};

class BlendshapeVisualizer : public GL3DCanvas {
  Q_OBJECT
public:
  BlendshapeVisualizer(QWidget *parent = 0);
  ~BlendshapeVisualizer();

  virtual QSize sizeHint() const {
    return QSize(600, 600);
  }

  virtual QSize minimumSizeHint() const {
    return QSize(600, 600);
  }

  void loadMesh(const string &filename);
  void loadReferenceMesh(const string &filename);

  QImage getImage() {
    return this->grabFrameBuffer();
  }

  QImage render() {
    repaint();
    return this->grabFrameBuffer();
  }

protected:
  virtual void paintGL();

  void enableLighting();
  void disableLighting();
  void computeDistance();
  void drawMesh(const BasicMesh &m);
  void drawMeshWithColor(const BasicMesh &m);
  void drawMeshVerticesWithColor(const BasicMesh& m);
  void drawColorBar(double, double);

private:
  vector<double> dists;
  BasicMesh mesh;
  BasicMesh refmesh;
};

class BlendshapeGeneration : public QMainWindow
{
  Q_OBJECT

public:
  BlendshapeGeneration(bool silent, QWidget *parent = 0);
  ~BlendshapeGeneration();

  virtual QSize sizeHint() const {
    return QSize(600, 600);
  }

  void LoadMeshes(const string& mesh, const string& refmesh) {
    if(silent) {
      ocanvas->loadMesh(mesh);
      ocanvas->loadReferenceMesh(refmesh);
    } else {
      canvas->loadMesh(mesh);
      canvas->loadReferenceMesh(refmesh);
      canvas->repaint();
    }
  }

  void LoadMesh(const string& mesh) {
    if(silent) {
      ocanvas->loadMesh(mesh);
    } else {
      canvas->loadMesh(mesh);
      canvas->repaint();
    }
  }

  void Save(const string& filename) {
    if(silent) {
      QImage pixmap = ocanvas->render();
      pixmap.save(filename.c_str());
    } else {
      QImage pixmap = canvas->render();
      pixmap.save(filename.c_str());
    }
  }

private slots:
  void slot_loadMesh();
  void slot_loadReferenceMesh();

private:
  Ui::BlendshapeGenerationClass ui;

  bool silent;
  BlendshapeVisualizer *canvas;
  OffscreenBlendshapeVisualizer *ocanvas;
};

#endif // BLENDSHAPEGENERATION_H
