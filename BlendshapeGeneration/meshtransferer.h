#ifndef MESHTRANSFERER_H
#define MESHTRANSFERER_H

#include "common.h"
#include <MultilinearReconstruction/basicmesh.h>
#include "Geometry/matrix.hpp"

class MeshTransferer
{
public:
  MeshTransferer();
  ~MeshTransferer();

  void setSource(const BasicMesh &src);
  void setTarget(const BasicMesh &tgt);
  void setStationaryVertices(const vector<int> &sv) { stationary_vertices = sv; }

  // transfer using a target shape
  BasicMesh transfer(const BasicMesh &S1);
  // transfer using a per-face deformation gradient
  BasicMesh transfer(const vector<PhGUtils::Matrix3x3d> &S1grad);

protected:
  void computeS0grad();
  void computeT0grad();

private:
  bool S0set, T0set;
  BasicMesh S0, T0;
  vector<PhGUtils::Matrix3x3d> S0grad, T0grad;
  vector<double> Ds;
  vector<int> stationary_vertices;
};

#endif // MESHTRANSFERER_H
