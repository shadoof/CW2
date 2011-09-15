/**
StudioModel

Loads 3DS models.
 */ 

#ifndef StudioModel_H
#define StudioModel_H

#define USE_VAR

#include <lib3ds/file.h>                        
#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>

//#include <G3D/BoundsTrait.h>


typedef ReferenceCountedPointer<class StudioModel> StudioModelRef;
typedef ReferenceCountedPointer<class PosedStudioModelObject> PosedStudioModelObjectRef;


class StudioModel : public ReferenceCountedObject
{
public:
  class Object;
  class Face;	
  
  typedef Object* ObjectRef; 

  static const Vector3&         getIgnoreVector() { return ignoreVector; }


private:

  // temp variables
  static std::string          ignoreString;
  static Vector3              ignoreVector;

  // temp classes
  class _collisionData
  {
  public:
    Vector3 contactPoint;
    Vector3 contactNormal;
    int     triangleIndex;

    _collisionData() {}
    _collisionData(Vector3 &cP, Vector3 &cN, int tI)
    {
      contactPoint = cP;
      contactNormal = cN;
      triangleIndex = tI;
    }
  };

#ifndef USE_VAR
    class Tri      
    {
    public:
      Tri(int p0, int p1, int p2)
      {
	index[0] = p0;
	index[1] = p1;
	index[2] = p2;
      }
      Tri()
      {
	index[0] = index[1] = index[2] = 0;
      }

        int                 index[3];
    };
#endif

  //    MeshAlg::Geometry       geometry;
  // Array<Vector2>          texCoordArray;
  //  Array<Vector3>          faceNormalArray;
  //  Array<Triangle>         triangleArray;
  /*
  class Edge {
  public:
    int vertexIndex[2];
  };
  */

  class Material
  {
  public:
    std::string name;
    double specularCoefficient;
    double shininess;
    Color4 color;
    Color3 ambient;
    Array<TextureRef> texture;

    Material() : color(1, 1, 1, 1),
		 ambient(0, 0, 0),
		 shininess(1),
		 specularCoefficient(1)
    {
    }
  };

  // Temporary arrays used to make sure duplicate textures are not loaded
  Array<std::string> loadedTextures;
  Array<TextureRef> loadedTextureRefs;

  // Objects loaded
  Array<StudioModel::Object> objects;
  Table<std::string, StudioModel::Object*> objectTable;
  
  // BSP used for collision detection
  KDTree<StudioModel::Face> triangleSoup;

  // Total number of vertices and faces in the model
  int nVertices;
  int nFaces;

  // Bounding box and sphere
  AABox _bbox;
  Vector3 _locus;
  Sphere _bsphere;

  // Base path to the model
  std::string _path;

  /**
     An edge is broken if it appears in
     only one face or if its faces do not
     contain the vertices (happens with colocated vertices)
  */
  Array<MeshAlg::Edge>     brokenEdgeArray;
  Array<MeshAlg::Edge>     edgeArray;

  /** 3D Studio */
  void parseMesh(Lib3dsMesh *mesh);
  void parseNode(Lib3dsNode *p, Matrix4 &frame); //Deprecated
  void load3DS(const std::string& filename);
  Lib3dsFile *file;

  /** Wavefront OBJ */
  void parseOBJMats(const std::string& filename, Array<Material>& mats); 
  void loadOBJ(const std::string& filename);
  
  // Model attributes
  bool _twoSided;
  double _scale;

  // Camera starting position (loaded from the 3DS file)
  Vector3 _startPos;

  /** Configure from a builder */
  //void set(MeshBuilder& builder);

public:
  class Object
  {
  public:
    std::string       name;
    MeshAlg::Geometry geometry;
    TextureRef        texture;
    Array<Vector2>    texCoordArray;
    Array<Vector3>    faceNormalArray;
    Array<Vector3>    vertNormalArray;
#ifdef USE_VAR
    Array<int>        triangleArray;
    VAR vertexArray;
    VAR normalArray;
    VAR texArray;
#else
    Array<Tri>        triangleArray;
#endif

    // MeshAlg data structures
    Array<MeshAlg::Edge> edges;
    Array<MeshAlg::Face> faces;
    Array<MeshAlg::Vertex> vertices;
    Array<MeshAlg::Face> weldedFaces;
    Array<MeshAlg::Edge> weldedEdges;
    Array<MeshAlg::Vertex> weldedVertices;
    int numBoundaryEdges;
    int numWeldedBoundaryEdges;
    
    Vector3      center;
    Sphere       bsphere;
    AABox        bbox;
    unsigned int hcode;
    
    // Material properties
    // TODO: Should probably move this into a separate structure
    Color3 ambient;
    Color4 diffuse;
    Color3 specular;
    float  shininess;

    PosedStudioModelObjectRef pose(const CoordinateFrame& cframe);

  };

  class Face : public G3D::Triangle
  {
  public:
    Object *parent;

    Face() : G3D::Triangle()
    {}
    Face(Vector3 &v1, Vector3 &v2, Vector3 &v3);
  };


  // Currently ignored
  std::string             name;

  /**@param twoSided When true, model is rendered without face culling
     @param buildBSP When true, a BSP is built for collision detection */
  StudioModel(const std::string &base, const std::string& filename, double scale = 1, bool twoSided = false, bool buildBSP=true);
  StudioModel() {}
  ~StudioModel();

  Vector3 getStartingPos() {return _startPos; }
  StudioModel::Object *getObjectByName(const std::string& name);
  Array<StudioModel::Object>::Iterator beginObjectIter() { return objects.begin(); }
  Array<StudioModel::Object>::Iterator endObjectIter() { return objects.end(); }

  void pose(const CoordinateFrame &cframe, Array<PosedStudioModelObjectRef>& posedObjects);
  PosedStudioModelObjectRef poseByName(const std::string& name, const CoordinateFrame &cframe);

  bool slideCollision(Vector3 &pos, float radius, const Vector3 &vec, Vector3 &slideVec = ignoreVector, std::string &objectName = ignoreString, int bounces=3);
  bool bounceCollision(Vector3 &pos, float radius, const Vector3 &vec, Vector3 &bounceDir = ignoreVector, std::string &objectName = ignoreString, int bounces=3);
  bool intersects(const Vector3 &pos, const Vector3 &dir, Vector3 &point = ignoreVector, Vector3 &normal = ignoreVector, std::string &objectName = ignoreString);
  

  void getObjectSpaceBoundingBox(AABox &box)
  {
    box = _bbox;
  }

  AABox objectSpaceBoundingBox()
  {
    return _bbox;
  }

  void getObjectSpaceBoundingSphere(Sphere &sph)
  {
    sph = _bsphere;
  }

  Sphere objectSpaceBoundingSphere()
  {
    return _bsphere;
  }

  void getObjectSpaceCenter(Vector3 &center)
  {
    center = _locus;
  }

  Vector3 objectSpaceCenter()
  {
    return _locus;
  }

    /**
     Render the model.
    */
  void render(RenderDevice *RD, float alpha = 1.f);
  // Renders the model and overrides material diffuse color
  void render(RenderDevice *RD, const Color4& color);

  static void render(RenderDevice *RD, StudioModel::Object *obj);

  int numVertices() const {
    return nVertices;
  }

  int numFaces() const {
    return nFaces;
  }

  //void getTriangleArray(Array<Triangle> &triArray)
  // {
  //   triArray = triangleArray;
  // }

  //int numBrokenEdges() const {
  //     return brokenEdgeArray.size();
  // }
};

template <> struct HashTrait<StudioModel::Face> {
    static unsigned int hashCode(const StudioModel::Face& face)
    {
        return face.hashCode();
    }
};

template<> struct BoundsTrait<class StudioModel::Face> {
    static void getBounds(const StudioModel::Face& face, G3D::AABox& out) { face.getBounds(out); }
};


class PosedStudioModelObject : public G3D::PosedModel
{
protected:
	StudioModel::Object *_objectRef;
	CoordinateFrame _cframe;
	GMaterial _material;
	bool _useMat;

public:
	PosedStudioModelObject(StudioModel::Object *objectRef, const CoordinateFrame& pframe, const GMaterial& mat, bool useMat);
	std::string name() const;
	void getCoordinateFrame(CoordinateFrame& c) const;
	const MeshAlg::Geometry& objectSpaceGeometry() const;
	const Array<MeshAlg::Face>& faces() const;
	const Array<MeshAlg::Edge>& edges() const;
	const Array<MeshAlg::Vertex>& vertices() const;
	const Array<MeshAlg::Face>& weldedFaces() const;
	const Array<MeshAlg::Edge>& weldedEdges() const;
	const Array<MeshAlg::Vertex>& weldedVertices() const;
	const Array<int>& triangleIndices() const;
	void getObjectSpaceBoundingSphere(Sphere&) const;
	void getObjectSpaceBoundingBox(Box&) const;
        const Array<Vector3>& objectSpaceFaceNormals(bool normalize=true) const;
	void render(class RenderDevice* renderDevice) const;
	int numBoundaryEdges() const;
	int numWeldedBoundaryEdges() const;
};


#endif
