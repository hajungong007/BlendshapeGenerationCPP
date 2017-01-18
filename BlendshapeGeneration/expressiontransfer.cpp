#include "common.h"

#include "blendshapegeneration.h"
#include <QtWidgets/QApplication>

#include "testcases.h"

#include <MultilinearReconstruction/basicmesh.h>
#include <MultilinearReconstruction/costfunctions.h>
#include <MultilinearReconstruction/ioutilities.h>
#include <MultilinearReconstruction/multilinearmodel.h>
#include <MultilinearReconstruction/parameters.h>
#include <MultilinearReconstruction/utils.hpp>
#include <MultilinearReconstruction/OffscreenMeshVisualizer.h>
#include <MultilinearReconstruction/meshvisualizer.h>

#include "meshdeformer.h"
#include "meshtransferer.h"
#include "cereswrapper.h"

#include "Geometry/matrix.hpp"
#include "triangle_gradient.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <boost/timer/timer.hpp>

void expressionTransfer(const string& res_filename, const string& blendshapes_path) {
  // Load the reconstruction results
  auto recon_results = LoadReconstructionResult(res_filename);

  // Read in all blendshapes
  const int nshapes = 46;
  vector<BasicMesh> B(nshapes+1);
  #pragma omp parallel for
  for(int i=0;i<=nshapes;++i) {
    B[i] = BasicMesh(blendshapes_path + "/B_" + std::to_string(i) + ".obj");
  }

  // Create the transferred shape
  BasicMesh mesh = B[0];
  for(int i=1;i<=nshapes;++i) {
    auto dBi = B[i].vertices() - B[0].vertices();
    mesh.vertices() += dBi * recon_results.params_model.Wexp_FACS[i];
  }

  mesh.Write("exp_transferred.obj");

  MultilinearModel model("/home/phg/Data/Multilinear/blendshape_core.tensor");
  MultilinearModelPrior model_prior;
  model_prior.load("/home/phg/Data/Multilinear/blendshape_u_0_aug.tensor",
                   "/home/phg/Data/Multilinear/blendshape_u_1_aug.tensor");


  model.ApplyWeights(recon_results.params_model.Wid, recon_results.params_model.Wexp);
  BasicMesh mesh0("/home/phg/Data/Multilinear/template.obj");
  mesh0.UpdateVertices(model.GetTM());
  mesh0.ComputeNormals();

  // Render the transferred mesh
  // for each image bundle, render the mesh to FBO with culling to get the visible triangles
  #if 0
  OffscreenMeshVisualizer visualizer(recon_results.params_cam.image_size.x, recon_results.params_cam.image_size.y);
  visualizer.SetMVPMode(OffscreenMeshVisualizer::CamPerspective);
  visualizer.SetRenderMode(OffscreenMeshVisualizer::Mesh);
  visualizer.BindMesh(mesh);
  visualizer.SetCameraParameters(recon_results.params_cam);
  visualizer.SetMeshRotationTranslation(recon_results.params_model.R, recon_results.params_model.T);
  visualizer.SetIndexEncoded(false);

  QImage img = visualizer.Render();
  img.save("transferred.png");
  #else
  {
    MeshVisualizer* w = new MeshVisualizer("Reconstruction result", mesh0);
    w->SetMeshRotationTranslation(recon_results.params_model.R, recon_results.params_model.T);
    w->SetCameraParameters(recon_results.params_cam);

    float scale = 640.0 / recon_results.params_cam.image_size.y;
    w->resize(recon_results.params_cam.image_size.x * scale, recon_results.params_cam.image_size.y * scale);
    w->show();
  }

  {
    MeshVisualizer* w = new MeshVisualizer("Transferred result", mesh);
    w->SetMeshRotationTranslation(recon_results.params_model.R, recon_results.params_model.T);
    w->SetCameraParameters(recon_results.params_cam);

    float scale = 640.0 / recon_results.params_cam.image_size.y;
    w->resize(recon_results.params_cam.image_size.x * scale, recon_results.params_cam.image_size.y * scale);
    w->show();
  }
  #endif
}

void printUsage(const string& program_name) {
  cout << "Usage: " << program_name << " from_exp" << endl;
}

void create_mesh(const string& filename) {
  auto recon_results = LoadReconstructionResult(filename);
  MultilinearModel model("/home/phg/Data/Multilinear/blendshape_core.tensor");
  MultilinearModelPrior model_prior;
  model_prior.load("/home/phg/Data/Multilinear/blendshape_u_0_aug.tensor",
                   "/home/phg/Data/Multilinear/blendshape_u_1_aug.tensor");


  model.ApplyWeights(recon_results.params_model.Wid, recon_results.params_model.Wexp);
  BasicMesh mesh0("/home/phg/Data/Multilinear/template.obj");
  mesh0.UpdateVertices(model.GetTM());
  mesh0.ComputeNormals();

  cout << recon_results.params_model.R << endl;

  glm::dmat4 Rmat = glm::eulerAngleYXZ(-recon_results.params_model.R[0],
                                       -recon_results.params_model.R[1],
                                       -recon_results.params_model.R[2]);
  Rmat = glm::transpose(Rmat);
  for(int i=0;i<mesh0.NumVertices();++i) {
    auto v = mesh0.vertex(i);
    glm::dvec4 vi = Rmat * glm::dvec4(v[0], v[1], v[2], 1);
    mesh0.set_vertex(i, Vector3d(vi.x, vi.y, vi.z));
  }
  mesh0.Write("created.obj");
}

int main(int argc, char** argv) {
  QApplication app(argc, argv);

  google::InitGoogleLogging(argv[0]);

  if( argc < 2 ) {
    printUsage(argv[0]);
  } else if (string(argv[1]) == "-c" ){
    create_mesh(argv[2]);
    return 0;
  } else {
    expressionTransfer(argv[1], argv[2]);
    return app.exec();
  }
}
