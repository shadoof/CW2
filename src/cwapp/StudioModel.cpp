/**
StudioModel
 */
#include "main.h"
#include "StudioModel.h"

//RenderDevice* renderDevice;
//extern GCamera       camera;
//extern CFontRef      font;

// temp variables
std::string          StudioModel::ignoreString;
Vector3              StudioModel::ignoreVector;

// Defines
#define EPSILON 0.00001

static double gaussian2D(double x, double z) {
    return exp(-4 * (square(x) + square(z)));
}


static double bump2D(double x, double z) {
  double t = G3D::min(G3D::pi(), 7 * sqrt(square(x) + square(z)) / sqrt(0.5));
    return cos(t) * .15;
}

static double saddle2D(double x, double z) {
    return (square(x*1.5) - square(z*1.5)) * .75;
}

static double flat2D(double x, double z) {
    return 0.0;
}


static double sin2D(double x, double z) {
    static double angle = toRadians(-25);
    double t = G3D::twoPi() * (cos(angle) * x + sin(angle) * z);
    return sin(t * 2) * 0.1;
}

static double lumpy2D(double x, double z) {
    x *= G3D::twoPi();
    z *= G3D::twoPi();
    return 
        (cos(x) * 0.1 + cos(2*x + 1) * 0.15 + cos(3*x - 2) * 0.1 + cos(5*x + 4) * 0.05 - x * .1 +
         cos(z) * 0.2 + cos(2.5*z - 1) * 0.15 + cos(4*z + 2) * 0.15 + cos(5*z + 4) * 0.05 - z * .1) / 2;
}


static double cliff2D(double x, double z) {
	if (x + z > .25) {
		return 0.5;
	} else {
		return 0.0;
	}
}

StudioModel::Face::Face(Vector3 &v1, Vector3 &v2, Vector3 &v3) : G3D::Triangle(v1, v2, v3)
{
}

StudioModel::StudioModel(const std::string &base, const std::string& filename, double scale, bool t, bool buildBSP) : 
  _twoSided(t),
  _scale(scale),
  nVertices(0),
  nFaces(0),
  _startPos(),
  _locus(Vector3::zero())
{
  _path = base;
  
  if (!fileExists(_path + filename)) 
  {
    //error("Critical Error", std::string("File not found: \"") + filename + "\"", true);
    exit(-1);
  }

  // Initialize arrays
  loadedTextures.resize(0);
  loadedTextureRefs.resize(0);
  objects.resize(0);
  triangleSoup.clear();

  

  //createHalfGear(); return;
  //createGrid(flat2D, 1024, true); return;
  //createGrid(lumpy2D, 1024, true); return;
  //createGrid(cliff2D, 1024, true); return;
  //createIsoGrid(cliff2D, 1024); return;
  //createGrid(bump2D, 900, true); return;
  //createIsoGrid(lumpy2D, 800); return;
  //createGrid(lumpy2D, 900, true); return;
  //createPolygon(); return;
  //createRing();  return;
  
  //    if (endsWith(f, ".ifs")) {
  //     loadIFS(filename);
  //} else if (endsWith(f, ".md2")) {
  //    loadMD2(filename);
  //} else if (endsWith(f, ".3ds")) {
    //} else if (endsWith(f, ".obj")) {
  //loadOBJ(filename);
  //} else if (endsWith(f, ".sm")) {
  //loadSM(filename);
  //} else {
  //debugAssert(false);
  //}

  // Load the model
  cout << "Loading " << filename << "..." << endl;

  std::string f = toLower(filename);
  if (endsWith(f, ".3ds"))
  {
    load3DS(_path + filename);
  } else if (endsWith(f, ".obj")) 
  {
    loadOBJ(_path + filename);
  }

  // Count vertices and faces, and calculate object center
  _locus = Vector3::zero();
  for(int i = 0; i < objects.size(); i++)
  {
    objects[i].center = Vector3::zero();
    for(int j = 0; j < objects[i].geometry.vertexArray.size(); j++)
    {
      objects[i].center += objects[i].geometry.vertexArray[j];
      _locus += objects[i].geometry.vertexArray[j];
    }
    
    if(objects[i].geometry.vertexArray.size() > 0)
    {
      // Calculate bounding sphere
      objects[i].center /= objects[i].geometry.vertexArray.size();
      
      float BSradius = 0;
      for(int j = 0; j < objects[i].geometry.vertexArray.size(); j++)
      {
      	float vertDist = (objects[i].geometry.vertexArray[j] - objects[i].center).squaredLength();
        if(vertDist > BSradius)
		BSradius = vertDist;
      }

      objects[i].bsphere = Sphere(objects[i].center, sqrt(BSradius));
    }

    nVertices += objects[i].geometry.vertexArray.size();
#ifndef USE_VAR
    nFaces += objects[i].triangleArray.size();
#else
    nFaces += objects[i].triangleArray.size()/3;
#endif
  }
  if(nVertices > 0)
  {
    _locus /= nVertices;
  }

  // Add the objects to the table for access by name
  // Also, calculate bounding sphere and box
  float maxRadius = 0;
  Vector3 max(-Vector3::inf()), min(Vector3::inf());
  for(int i = 0; i < objects.size(); i++)
  {
    // Set the object's hash code
    objects[i].hcode = i;
    // Add the object to the table
    objectTable.set(objects[i].name, &objects[i]);

    // Calculate adjacencies and welds
    MeshAlg::computeAdjacency(objects[i].geometry.vertexArray, objects[i].triangleArray,
			      objects[i].faces, objects[i].edges, objects[i].vertices);
    if(objects[i].faceNormalArray.size() == 0)
    {
    	MeshAlg::computeNormals(objects[i].geometry.vertexArray, objects[i].faces, objects[i].vertices,
	   		        objects[i].vertNormalArray, objects[i].faceNormalArray);
    } else
    {
	objects[i].vertNormalArray = objects[i].geometry.normalArray;
    }
    objects[i].weldedFaces = objects[i].faces;
    objects[i].weldedEdges = objects[i].edges;
    objects[i].weldedVertices = objects[i].vertices;
    MeshAlg::weldAdjacency(objects[i].geometry.vertexArray, 
			   objects[i].weldedFaces, objects[i].weldedEdges, objects[i].weldedVertices);
    objects[i].numBoundaryEdges = MeshAlg::countBoundaryEdges(objects[i].edges);
    objects[i].numWeldedBoundaryEdges = MeshAlg::countBoundaryEdges(objects[i].weldedEdges);
    
    Vector3 objMin(Vector3::inf()), objMax(-Vector3::inf());
    // Calculate bounds
    for(int j = 0; j < objects[i].geometry.vertexArray.size(); j++)
    {
      // Sphere
      float vertDist = (objects[i].geometry.vertexArray[j]-_locus).squaredLength();
      if(vertDist > maxRadius)
      {
	maxRadius = vertDist;
      }

      // Calculate bounding boxes
      Vector3 gVec = objects[i].geometry.vertexArray[j];
      
      // For entire model
      if(gVec.x > max.x)
      {
	max.x = gVec.x;
      } else if(gVec.x < min.x)
      {
	min.x = gVec.x;
      }
      if(gVec.y > max.y)
      {
	max.y = gVec.y;
      } else if(gVec.y < min.y)
      {
	min.y = gVec.y;
      }
      if(gVec.z > max.z)
      {
	max.z = gVec.z;
      } else if(gVec.z < min.z)
      {
	min.z = gVec.z;
      }

      // For the object
      if(gVec.x > objMax.x)
      {
	objMax.x = gVec.x;
      } else if(gVec.x < objMin.x)
      {
	objMin.x = gVec.x;
      }
      if(gVec.y > objMax.y)
      {
	objMax.y = gVec.y;
      } else if(gVec.y < objMin.y)
      {
	objMin.y = gVec.y;
      }
      if(gVec.z > objMax.z)
      {
	objMax.z = gVec.z;
      } else if(gVec.z < objMin.z)
      {
	objMin.z = gVec.z;
      }
    }
    //cout << "BBOX MAX: " << objMax << "\tBBOX MIN: " << objMin << endl;
    objects[i].bbox = AABox(objMin, objMax);
  }
  _bbox = AABox(min, max);
  _bsphere = Sphere(_locus, sqrt(maxRadius));
    
  // Load the collision detection BSP
  if(buildBSP)
  {
    // Build triangle soup for collision detection
    cout << "Building BSP...";
#ifndef USE_VAR
    int *indices;
#else
    int indices[3];
    int tIndex;
#endif
    for(int i = 0; i < objects.size(); i++)
    {
#ifndef USE_VAR
      for(int j = 0; j < objects[i].triangleArray.size(); j++)
      {
	indices = objects[i].triangleArray[j].index;
#else
      for(int j = 0; j < objects[i].triangleArray.size()/3; j++)
      {
	tIndex = j*3;
	indices[0] = objects[i].triangleArray[tIndex];
	indices[1] = objects[i].triangleArray[tIndex+1];
	indices[2] = objects[i].triangleArray[tIndex+2];
#endif
	StudioModel::Face face(objects[i].geometry.vertexArray[indices[0]],
			       objects[i].geometry.vertexArray[indices[1]],
			       objects[i].geometry.vertexArray[indices[2]]);
	face.parent = &objects[i];
      
        triangleSoup.insert(face);
      }
    }
    triangleSoup.balance();
    cout << "done!" << endl;
  }

  // Print model information
  cout << "Faces: " << nFaces << "\tVertices: " << nVertices << endl;
  cout << "Start pos: " << _startPos << endl << endl;
}

StudioModel::~StudioModel()
{
  for(int i = 0; i < objects.size(); i++)
  {
    objects[i].texture = NULL;
  }
}

class RayIntersectCallback
    {
    public:
        const Vector3 &pos, &vec;
        Vector3 &point, &normal;
        Vector3 cPoint, cNormal;
        const StudioModel::Face *selectedFace;

        RayIntersectCallback(const Vector3& _pos, const Vector3& _vec, Vector3& _point, Vector3& _normal)
            : pos(_pos), vec(_vec), point(_point), normal(_normal)
        {
            selectedFace = NULL;
        }

        void operator () (const Ray& ray, const StudioModel::Face& obj, float& t)
        {
            double time = CollisionDetection::collisionTimeForMovingPointFixedTriangle(
                pos,
                vec, 
                obj, 
                cPoint, 
                cNormal);      

            if(time <= 1+EPSILON)
            {
                if(time < t)
                {
                    t = (float)time;

                    selectedFace = &obj;
                    if(&point != &StudioModel::getIgnoreVector())
                    {
                        point = cPoint;
                    }
                    if(&normal != &StudioModel::getIgnoreVector())
                    {
                        normal = cNormal;
                    }
                }
            }
        }
    };

bool StudioModel::intersects(const Vector3 &pos, const Vector3 &vec, Vector3 &point, Vector3 &normal, std::string &objectName)
{
    Ray ray = Ray::fromOriginAndDirection(pos, vec.direction());

    float t = G3D::inf();

    RayIntersectCallback iter(pos, vec, point, normal);
    triangleSoup.intersectRay(ray, iter, t);

    //typedef AABSPTree<StudioModel::Face>::RayIntersectionIterator IT;
    //const IT end = triangleSoup.endRayIntersection();

    //for (IT obj = triangleSoup.beginRayIntersection(ray); obj != end; ++obj) 
    //{
    //    double time = CollisionDetection::collisionTimeForMovingPointFixedTriangle(
    //        pos,
    //        vec, 
    //        *obj, 
    //        cPoint, 
    //        cNormal);
    //    if(time <= 1+EPSILON)
    //    {
    //        if(time < t)
    //        {
    //            t = time;

    //            selectedFace = &*obj;
    //            if(&point != &ignoreVector)
    //            {
    //                point = cPoint;
    //            }
    //            if(&normal != &ignoreVector)
    //            {
    //                normal = cNormal;
    //            }
    //        }
    //    }
    //}

    if(t == G3D::inf())
    {
        return false;
    } 

    if(&objectName != &ignoreString)
    {
        objectName = iter.selectedFace->parent->name;
    }

    return true;
}

bool StudioModel::slideCollision(Vector3 &pos, float radius, const Vector3 &vec, Vector3 &slideVec, std::string &objectName, int bounces)
{
  if(vec.squaredLength() == 0)
  {
    return false;
  }

  Sphere player(pos, radius);
  double t = G3D::inf();
  double time;
  int selectedTri = -1;
  Vector3 outLocation;
  Vector3 outNormal;
  Vector3 collisionPoint;
  Vector3 collisionNormal;

  float temp;

  Vector3 min = pos;
  Vector3 max = pos + vec;
  if(min.y <= max.y)
  {
    min.y -= radius;
    max.y += radius;
  } else
  {
    min.y += radius;
    max.y -= radius;
  }
  if(min.x <= max.x)
  {
    min.x -= radius;
    max.x += radius;
  } else
  {
    min.x += radius;
    max.x -= radius;
  }
  if(min.z <= max.z)
  {
    min.z -= radius;
    max.z += radius;
  } else
  {
    min.z += radius;
    max.z -= radius;
  }
  if(min.x > max.x)
  {
    temp = min.x;
    min.x = max.x;
    max.x = temp;
  }
  if(min.y > max.y)
  {
    temp = min.y;
    min.y = max.y;
    max.y = temp;
  }
  if(min.z > max.z)
  {
    temp = min.z;
    min.z = max.z;
    max.z = temp;
  }
  
  AABox intersect(min, max);
  Array<StudioModel::Face> relevantTris;
  triangleSoup.getIntersectingMembers(intersect, relevantTris);
  for(int i = 0; i < relevantTris.size(); i++)
  {
    time = CollisionDetection::collisionTimeForMovingSphereFixedTriangle(player,
									 vec,
									 relevantTris[i],      
									 outLocation,
									 outNormal);

    if(time <= (1+EPSILON))
    {
      if(time < t)
      {
	t = time;

	selectedTri = i;
	collisionPoint = outLocation;
	collisionNormal = outNormal;
      
	if(t <= 0)
	{
	  // We definitely have a collision here
	  break;
	}
      }
    }
  }

  if(t == G3D::inf())
  {
    //cout << "No collision!" << endl;
    pos = pos + vec;
    
    if(&objectName != &ignoreString)
    {
      objectName += "";
    }

    return false;
  }

  objectName += relevantTris[selectedTri].parent->name + " ";

  //Vector3 normVec = vec.direction();
  Vector3 slideDir = vec.length()*CollisionDetection::slideDirection(player, vec, t, collisionPoint);
  
  pos = collisionPoint + (collisionNormal*(radius+2*EPSILON));
  //pos + (t * vec);
  if(&slideVec != &ignoreVector)
  {
    if(bounces > 0)
    {
      Vector3 slidePos = pos;
      Vector3 newSlideVec;
      //slideVec = (1-t)*vec.length()*slideDir;
      if(slideCollision(slidePos, radius, slideDir, newSlideVec, objectName, bounces-1))
      {
	slideVec = (slidePos + newSlideVec) - pos;
      } else
      {
	slideVec = slideDir;
      }
    } else
    {
      slideVec = slideDir;
    }
  }

  return true;
}

bool StudioModel::bounceCollision(Vector3 &pos, float radius, const Vector3 &vec, Vector3 &bounceDir, std::string &objectName, int bounces)
{
  if(vec.squaredLength() == 0)
  {
    return false;
  }

  Sphere player(pos, radius);
  double t = G3D::inf();
  double time;
  int selectedTri = -1;
  Vector3 outLocation;
  Vector3 outNormal;
  Vector3 collisionPoint;
  Vector3 collisionNormal;

  Vector3 min = pos;
  Vector3 max = pos + vec;
  float temp;
  if(min.y <= max.y)
  {
    min.y -= radius;
    max.y += radius;
  } else
  {
    min.y += radius;
    max.y -= radius;
  }
  if(min.x <= max.x)
  {
    min.x -= radius;
    max.x += radius;
  } else
  {
    min.x += radius;
    max.x -= radius;
  }
  if(min.z <= max.z)
  {
    min.z -= radius;
    max.z += radius;
  } else
  {
    min.z += radius;
    max.z -= radius;
  }
  if(min.x > max.x)
  {
    temp = min.x;
    min.x = max.x;
    max.x = temp;
  }
  if(min.y > max.y)
  {
    temp = min.y;
    min.y = max.y;
    max.y = temp;
  }
  if(min.z > max.z)
  {
    temp = min.z;
    min.z = max.z;
    max.z = temp;
  }
  
  AABox intersect(min, max);
  Array<StudioModel::Face> relevantTris;
  triangleSoup.getIntersectingMembers(intersect, relevantTris);
  for(int i = 0; i < relevantTris.size(); i++)
  {
    time = CollisionDetection::collisionTimeForMovingSphereFixedTriangle(player,
									 vec,
									 relevantTris[i],      
									 outLocation,
									 outNormal);

    if(time <= (1+EPSILON))
    {
      if(time < t)
      {
	t = time;

	selectedTri = i;
	collisionPoint = outLocation;
	collisionNormal = outNormal;
      
	if(t <= 0)
	{
	  // We definitely have a collision
	  break;
	}
      }
    }
  }

  if(t == G3D::inf())
  {
    //cout << "No collision!" << endl;
    pos = pos + vec;

    if(&objectName != &ignoreString)
    {
      objectName += "";
    }

    return false;
  }
  
  objectName += relevantTris[selectedTri].parent->name + " ";

  bounceDir = CollisionDetection::bounceDirection(player, vec, t, collisionPoint, collisionNormal);

  pos = pos + (t * vec);
  Vector3 bounceVec = (1-t)*vec.length()*bounceDir;
  
  if(bounces-1 > 0)
  {
    Vector3 newBounceDir;
    if(bounceCollision(pos, radius, bounceVec, newBounceDir, objectName, bounces-1))
    {
      bounceDir = newBounceDir;
    }
  } else
  {
    pos += bounceVec;
  }
  return true;
}

std::string makeLowerCase(const std::string& str)
{
  //cout << "UC: " << str << "\t";
  std::string retStr = str;

  for(int i = 0; i < str.length(); i++)
  {
    retStr[i] = tolower(str[i]);
  }

  //cout << "LC: " << retStr << endl;
  return retStr;
}

std::string stripExt(const std::string& filename)
{
  for(int i = 0; i < filename.length(); i++)
  {
    if(filename[i] == '.')
    {
      return filename.substr(0, i);
    }
  }

  return "";
}

void parsePath(const std::string& filename, Array<std::string>& path, std::string& base)
{
  int position = 0;
  int strStart = 0;
  while(position < filename.length())
  {
    if(filename[position] == '/' || filename[position] == '\\')
    {
      path.append(filename.substr(strStart, position-strStart));
      strStart = position+1;
      position++;
    }

    position++;
  }

  base = filename.substr(strStart, filename.length()-strStart);
}

TextureRef loadLCTexture(const std::string& b, const std::string& filename)
{ 
  std::string drive, base;
  Array<std::string> path;

  // Parse the subdirectories
  parsePath(filename, path, base);

  // Find the matching subdirectory
  std::string searchPath = b;
  for(int i = 0; i < path.size(); i++)
  {
    Array<std::string> dirs;
    getDirs(searchPath + "*", dirs);
    for(int j = 0; j < dirs.size(); j++)
    {
      if(makeLowerCase(dirs[j]) == makeLowerCase(path[i]))
      {
	searchPath += dirs[j] + "/";
	break;
      }
    }
  }

  std::string finalPath = searchPath + "*.*";

  Array<std::string> matchFiles;
  getFiles(finalPath, matchFiles);
  
  //cout << "File to match: " << makeLowerCase(base) << endl;
  for(int i = 0; i < matchFiles.size(); i++)
  {
    //cout << makeLowerCase(stripExt(matchFiles[i]));
    std::string ext = makeLowerCase(filenameExt(matchFiles[i]));
    if((ext == "jpg") /*|| (ext == "tga")*/)
    {
      if(makeLowerCase(stripExt(matchFiles[i])) == makeLowerCase(base))
      {
	//cout << endl << "Texture matched!";
	//return loadBrightTexture(searchPath + matchFiles[i], 2.0);
	return Texture::fromFile(searchPath + matchFiles[i]);
      }
    }
  }

  return NULL;
}

void StudioModel::parseMesh(Lib3dsMesh *mesh)
{
  //debugAssert(mesh);
  if (!mesh)
  {
    return;
  }

  if(mesh->faces == 0)
  {
    return;
  }
  // node->user.d=glGenLists(1);
  //glNewList(node->user.d, GL_COMPILE);

  Object obj;

  {
    unsigned p;
    Lib3dsVector *normalL=(Lib3dsVector *)malloc(3*sizeof(Lib3dsVector)*mesh->faces);
	
    //float *piv = node->data.object.pivot;
    //float *c = &node->matrix[0][0];
    //Matrix4 nodeMatrix( c[0],    c[1],     c[2],   c[3],
    //	    c[4],    c[5],     c[6],   c[7],
    //	    c[8],    c[9],     c[10],  c[11],
    //	   -piv[0], -piv[1],  -piv[2], 1);
    //Matrix4 nodeMatrix(&node->matrix[0][0]);
    //{
      //Lib3dsMatrix M;
      //lib3ds_matrix_copy(M, mesh->matrix);
      //lib3ds_matrix_inv(M);
      //frame = frame * Matrix4(&M[0][0]);
      //frame = frame * Matrix4(&node->matrix[0][0]);
      //glMultMatrixf(&M[0][0]);
      //}
    lib3ds_mesh_calculate_normals(mesh, normalL);

    obj.geometry.vertexArray.resize(mesh->points);
    obj.geometry.normalArray.resize(mesh->points);
    obj.faceNormalArray.resize(mesh->faces);
    obj.texCoordArray.resize(mesh->points);
#ifndef USE_VAR
    obj.triangleArray.resize(mesh->faces);
#else
    obj.triangleArray.resize(mesh->faces*3);
#endif

    for(int verts = 0; verts < mesh->points; verts++)
    {
      // Rotate geometry to G3D coordinate system
      Vector3 vert(mesh->pointL[verts].pos);
      float z = vert.z;
      vert.z = -vert.y;
      vert.y = z;
      //vert = vert - Vertex3(node->data.object->pivot);
      //obj.geometry.vertexArray[verts] = (frame * Vector4(vert, 1)).xyz();
      //vert = (nodeMatrix * Vector4(vert, 1)).xyz();

      obj.geometry.vertexArray[verts] = _scale*vert;//Vector3(mesh->pointL[verts].pos);
      //obj.texCoordArray[verts] = Vector2(mesh->texelL[verts]);
    }

    for(int uv = 0; uv < mesh->texels; uv++)
    {
      //cout << "Texels found!" << endl;
      obj.texCoordArray[uv] = Vector2(mesh->texelL[uv]);
    }

    Lib3dsFace *f = &mesh->faceL[0];
    Lib3dsMaterial *mat = 0;
    if(f->material[0])
    {
      mat = lib3ds_file_material_by_name(file, f->material);
    }

    if(mat)
    {
      obj.name = mat->name;
      obj.ambient = Color3(0, 0, 0);
      mat->diffuse[3] = 1.f;
      obj.diffuse = Color4(mat->diffuse);
      obj.specular = Color3(mat->specular);
      obj.shininess = pow(2.0, 10.0*mat->shininess);
      if(obj.shininess > 128.0)
      {
	obj.shininess = 128.0;
      }

      obj.texture = NULL;
      std::string texName = mat->texture1_map.name;
      if(texName != "")
      {
	//cout << "Texture name: " << texName << endl;
	for(int lTex = 0; lTex < loadedTextures.size(); lTex++)
	{
	  if(loadedTextures[lTex] == texName)
	  {
	    obj.texture = loadedTextureRefs[lTex];
	    break;
	  }
	}
	    
	if(obj.texture.isNull())
	{
	  //if(fileExists(texName))
	  //{
	  // Load the texture from a file
	  //cout << "Loading the texture!" << endl;
	  cout << "\tLoading " << texName << "...";
	  obj.texture = loadLCTexture(_path, stripExt(texName));
	  if(obj.texture.notNull())
	  {
	    // Apparently, UV texture coordinates are inverted
	    obj.texture->invertY = true;
	    loadedTextures.append(texName);
	    loadedTextureRefs.append(obj.texture);
	    cout << "done!" << endl;
	  } else
	  {
	    cout << "failed!" << endl;
	  }
	  //}
	}
      }
    } else
    {
      //cout << "No material!" << endl;
      obj.name = mesh->name;
      obj.ambient = Color3(0, 0, 0);
      obj.diffuse = Color4(1, 1, 1, 1);
      obj.specular = Color3(1, 1, 1);
      obj.shininess = 1;
      obj.texture = NULL;
    }

    for (p=0; p<mesh->faces; ++p) 
    {
      Lib3dsFace *f=&mesh->faceL[p];
      {
	obj.faceNormalArray[p] = Vector3(f->normal);
	int indices[3];
	for(int i = 0; i < 3; ++i)
	{
	  indices[i] = f->points[i];
	  obj.geometry.normalArray[indices[i]] = Vector3(normalL[3*p+i]);
	}
#ifndef USE_VAR
	obj.triangleArray[p] = Tri(indices[0], indices[1], indices[2]);
#else
	int pIndex = p*3;
	obj.triangleArray[pIndex] = indices[0];
	obj.triangleArray[pIndex+1] = indices[1];
	obj.triangleArray[pIndex+2] = indices[2];
#endif
      }
    }

    free(normalL);
  }

#ifdef USE_VAR
  VARAreaRef area = the_app.var_static;
  obj.vertexArray = VAR(obj.geometry.vertexArray, area);
  obj.normalArray = VAR(obj.geometry.normalArray, area);
  obj.texArray    = VAR(obj.texCoordArray, area);
#endif

  objects.append(obj);
}

void StudioModel::load3DS(const std::string& filename) 
{
  //MeshBuilder builder(_twoSided);

  //cout << _path + filename << endl;
    file = lib3ds_file_load(filename.c_str());
    if(!file)
    {
      debugAssert(false);
    }

    // Set starting pos
    Lib3dsCamera *camera = file->cameras;
    if(camera)
    {
      _startPos = Vector3(camera->position);
    }
    

    Matrix4 frame;
    for(Lib3dsMesh *m = file->meshes; m != 0; m = m->next)
    {
      parseMesh(m);
    }
    //for(Lib3dsNode *p = file->nodes; p != 0; p = p->next)
    //{
    //  cout << "Parsing a node!" << endl;
    //  parseNode(p, frame);
    //}

    lib3ds_file_free(file);

    loadedTextures.resize(0);
    loadedTextureRefs.resize(0);

    /*

    BinaryInput b(filename, G3D_LITTLE_ENDIAN);

    Load3DS loader(b);

    for (int obj = 0; obj < loader.objectArray.size(); ++obj) {
        const Matrix4& keyframe = loader.objectArray[obj].keyframe;
        
        Vector3 pivot = loader.objectArray[obj].pivot;

        const Array<int>&     index   = loader.objectArray[obj].indexArray;
        const Array<Vector3>& vertex  = loader.objectArray[obj].vertexArray;

	const Array<Vector2>& texcoord = loader.objectArray[obj].textureArray;
       	for(int i = 0; i < texcoord.size(); i++)
       	{
	  texCoordArray.append(texcoord[i]);
	}

	//	for(int i = 0; i < vertex.size(); i++)
	//{
	// vertex[i] = _scale*vertex[i];
	//}

        // The cframe has already been applied in the 3DS file.  If
        // we change the Load3DS object representation to move
        // coordinates back to object space, this is the code
        // that will promote them to world space before constructing
        // the mesh.

        const Matrix4&        cframe  = loader.objectArray[obj].cframe;
        (void)cframe;
        //#define vert(V) (keyframe * cframe * Vector4(vertex[index[V]], 1)).xyz()

        // TODO: Figure out correct transformation
        #define vert(V) \
          ((keyframe * \
           ( Vector4(vertex[index[V]], 1) - Vector4(pivot, 0)) \
           )).xyz() 

        for (int i = 0; i < index.size(); i += 3) {
            builder.addTriangle(
                vert(i),
                vert(i + 1),
                vert(i + 2));
        }

        #undef vert
    }
      
    builder.setName(loader.objectArray[0].name);

    set(builder);
    */
}

void StudioModel::parseOBJMats(const std::string& filename, 
			       Array<Material>& mats)
{
  TextInput::Settings options;
  options.cppComments = false;
  options.otherCommentCharacter = '#';
  TextInput t = *(new TextInput(filename, options));

  int curMaterial = -1;

  try 
  {
    while (t.hasMore()) 
    {
      Token tag = t.read();

      if (tag.type() == Token::SYMBOL) 
      {
	if(tag.string() == "newmtl")
	{
	  std::string matName = t.readSymbol();

	  curMaterial++;
	  mats.resize(mats.size()+1);
	  mats[curMaterial].name = matName;
	} else if(tag.string() == "Ni")
	{
	  mats[curMaterial].shininess = t.readNumber();
	} else if(tag.string() == "Kd")
	{
	  mats[curMaterial].color.r = t.readNumber();
	  mats[curMaterial].color.g = t.readNumber();
	  mats[curMaterial].color.b = t.readNumber();
	} else if(tag.string() == "Ka")
	{
	  mats[curMaterial].ambient.r = t.readNumber();
	  mats[curMaterial].ambient.g = t.readNumber();
	  mats[curMaterial].ambient.b = t.readNumber();
	} else if(tag.string() == "map_Kd")
	{
	  std::string texName;
	  while(t.peek().line() == tag.line() && t.hasMore())
	  {
	    Token tok = t.read();
	    texName += tok.string();
	  }
	  TextureRef texture = NULL;

	  //cout << "Texture name: " << texName << endl;
	  for(int lTex = 0; lTex < loadedTextures.size(); lTex++)
	  {
	    if(loadedTextures[lTex] == texName)
	    {
	      //cout << "\tSetting cached texture " << texName << endl;
	      texture = loadedTextureRefs[lTex];
	      mats[curMaterial].texture.append(texture);
	      break;
	    }
	  }
	    
	  if(texture.isNull())
	  {
	    //if(fileExists(texName))
	    //{
	    // Load the texture from a file
	    //cout << "Loading the texture!" << endl;
	    cout << "\tLoading " << texName << "...";
	    texture = loadLCTexture(_path, stripExt(texName));
	    if(texture.notNull())
	    {
	      // Apparently, UV texture coordinates are inverted
	      texture->invertY = true;
	      loadedTextures.append(texName);
	      loadedTextureRefs.append(texture);
	      mats[curMaterial].texture.append(texture);
	      cout << "done!" << endl;
	    } else
	    {
	      cout << "failed!" << endl;
	    }
	  }
	}
      }
      // Read to the next line
      while ((t.peek().line() == tag.line()) && t.hasMore()) 
      {
	t.read();
      }
    }
  } catch (TextInput::WrongTokenType& e) 
  {
    debugAssert(e.expected != e.actual);
    (void)e;
  }

  loadedTextures.resize(0);
  loadedTextureRefs.resize(0);
}

struct OBJVertex
{
  int vIndex;
  int tIndex;
  int nIndex;
};

void StudioModel::loadOBJ(const std::string& filename)
{
  //cout << "Loading the OBJ file " << filename << endl;

  TextInput::Settings options;
  options.cppComments = false;
  options.otherCommentCharacter = '#';
  TextInput t(filename, options);

  Array<Vector3> va;
  Array<Vector3> vn;
  Array<Vector2> vt;
  //Array< Array<OBJVertex> > objectVertices;
  Array< Array<int> > oVI;
  Array< Array<int> > oTI;
  Array< Array<int> > oNI;

  Array<Material> mats;

  int curObject = -1;

  try 
  {
    while (t.hasMore()) 
    {
      //cout << "Reading tokens!" << endl;
      Token tag = t.read();

      if (tag.type() == Token::SYMBOL) 
      {
	if(tag.string() == "mtllib")
	{
	  // Material library
	  std::string matLib;
	  while(t.peek().line() == tag.line() && t.hasMore())
	  {
	    Token tok = t.read();
	    matLib += tok.string();
	  }
	  //cout << "\tReading a material library from " << matLib << "..." << endl;
	  parseOBJMats(_path + matLib, mats);
	} else if(tag.string() == "g")
	{
	  // Possibly a new group
	  std::string objName = t.readSymbol();
	  if(objName != "default")
	  {
	    //cout << "\tReading the group \"" << objName << "\"..." << endl;

	    //objects.resize(objects.size()+1);
	    //lObjs.resize(lObjs.size()+1);
	    //curObject++;

	    //objects[curObject].name = objName;
	  }
	} else if(tag.string() == "usemtl")
	{
	  // Find the current material
	  std::string mtlName = t.readSymbol();

	  int objIndex = -1;
	  for(int i = 0; i < objects.size(); i++)
	  {
	    if(objects[i].name == mtlName)
	    {
	      objIndex = i;
	      break;
	    }
	  }

	  if(objIndex >= 0)
	  {
	    // Add to an existing object
	    curObject = objIndex;
	  } else 
	  {
	    // Add a new object
	    objects.resize(objects.size()+1);
	    curObject = objects.size()-1;
	    objects[curObject].name = mtlName;

	    oVI.resize(objects.size());
	    oTI.resize(objects.size());
	    oNI.resize(objects.size());

	    int material = -1;
	    for(int i = 0; i < mats.size(); i++)
	    {
	      if(mats[i].name == mtlName)
	      {
		//cout << "\t\tUsing material \"" << mtlName << "\"..." << endl;
		material = i;
		objects[curObject].diffuse = mats[i].color;
		objects[curObject].ambient = mats[i].ambient;
		objects[curObject].specular = Color3(1, 1, 1);
		objects[curObject].shininess = mats[i].shininess;
		if(mats[i].texture.size() > 0)
		{
		  objects[curObject].texture = mats[i].texture[0];
		} else
		{
		  //cout << "NO TEXTURE!" << endl;
		  objects[curObject].texture = NULL;
		}
		break;
	      }
	    }
	    
	    // Set a default material if it doesn't exist
	    if(material == -1)
	    {
	      cout << "Using a default material" << endl;
	      mats.resize(mats.size()+1);
	      material = mats.size()-1;
	      mats[material].name = mtlName;

	      objects[curObject].diffuse = mats[material].color;
	      objects[curObject].ambient = mats[material].ambient;
	      objects[curObject].shininess = mats[material].shininess;
	      objects[curObject].texture = NULL;
	    }
	  }
	} else if(tag.string() == "v") 
	{
	  // Vertex
	  Vector3 vertex;
	  vertex.x = t.readNumber();
	  vertex.y = t.readNumber();
	  vertex.z = t.readNumber();

	  va.append(vertex);
	} else if(tag.string() == "vn")
	{
	  // Vertex normal
	  Vector3 normal;
	  normal.x = t.readNumber();
	  normal.y = t.readNumber();
	  normal.z = t.readNumber();

	  vn.append(normal);
	} else if(tag.string() == "vt")
	{
	  // Texture coordinate
	  Vector2 uv;
	  uv.x = t.readNumber();
	  uv.y = t.readNumber();

	  vt.append(uv);
	} else if (tag.string() == "f") 
	{
	  //cout << "\tReading some faces..." << endl;
	  
	  // Face (3 or more vertices)
	  // Read vertices
	  Array<OBJVertex> vertexIndex;
	  while(t.peek().line() == tag.line() && t.hasMore())
	  {
	    OBJVertex vert;
	    // Vertex index
	    vert.vIndex = ((int)t.readNumber())-1;
	    
	    // Texture index
	    if((t.peek().type() != Token::NUMBER) && (t.peek().line() == tag.line()) && t.hasMore())
	    {
	      t.readSymbol(); // '/'
	      vert.tIndex = ((int)t.readNumber())-1;
	    }

	    // Normal index
	    if((t.peek().type() != Token::NUMBER) && (t.peek().line() == tag.line()) && t.hasMore())
	    {
	      t.readSymbol(); // '/'
	      vert.nIndex = ((int)t.readNumber())-1;
	    }

	    vertexIndex.append(vert);
	  }
	  
	  // Assume it's a triangle fan from the first vertex.
	  // (The vertices indicate the silhouette of the face)
	  for(int i = 1; i < vertexIndex.size()-1; i++)
	  {
	    int vI = oVI[curObject].size();
	    //int i1 = (i % 2 == 1) ? 2 : 1;
	    //int i2 = (i % 2 == 1) ? 1 : 2;
	    
	    // Vertex 1
	    oVI[curObject].append(vertexIndex[0].vIndex);
	    oTI[curObject].append(vertexIndex[0].tIndex);
	    oNI[curObject].append(vertexIndex[0].nIndex);

	    // Vertex 2
	    oVI[curObject].append(vertexIndex[i].vIndex);
	    oTI[curObject].append(vertexIndex[i].tIndex);
	    oNI[curObject].append(vertexIndex[i].nIndex);
	    
	    // Vertex 3
	    oVI[curObject].append(vertexIndex[i + 1].vIndex);
	    oTI[curObject].append(vertexIndex[i + 1].tIndex);
	    oNI[curObject].append(vertexIndex[i + 1].nIndex);

	    objects[curObject].triangleArray.append(vI);
	    objects[curObject].triangleArray.append(vI+1);
	    objects[curObject].triangleArray.append(vI+2);
	  }

	  /*
	  INCORRECT - faces are not triangle strips
	  for(int i = 0; i < vertexIndex.size()-2; i++)
	  {
	    int vI = oVI[curObject].size();
	    int i1 = (i % 2 == 1) ? 2 : 1;
	    int i2 = (i % 2 == 1) ? 1 : 2;
	    
	    // Vertex 1
	    oVI[curObject].append(vertexIndex[i].vIndex);
	    oTI[curObject].append(vertexIndex[i].tIndex);
	    oNI[curObject].append(vertexIndex[i].nIndex);

	    // Vertex 2
	    oVI[curObject].append(vertexIndex[i + 1].vIndex);
	    oTI[curObject].append(vertexIndex[i + 1].tIndex);
	    oNI[curObject].append(vertexIndex[i + 1].nIndex);
	    
	    // Vertex 3
	    oVI[curObject].append(vertexIndex[i + 2].vIndex);
	    oTI[curObject].append(vertexIndex[i + 2].tIndex);
	    oNI[curObject].append(vertexIndex[i + 2].nIndex);

	    objects[curObject].triangleArray.append(vI);
	    objects[curObject].triangleArray.append(vI+i1);
	    objects[curObject].triangleArray.append(vI+i2);
	  }
	  */
	  
	  //cout << (vertexIndex.size()-2) << " faces read!" << endl;
	}
      }

      // Read to the next line
      while ((t.peek().line() == tag.line()) && t.hasMore()) 
      {
	t.read();
      }
    }
  } catch (TextInput::WrongTokenType& e) 
  {
    std::string errorMsg;
    switch(e.expected)
    {
    case Token::STRING:
      errorMsg += "string expected, ";
      break;
    case Token::SYMBOL:
      errorMsg += "Symbol expected, ";
      break;
    case Token::NUMBER:
      errorMsg += "Number expected, ";
      break;
    case Token::END:
      errorMsg += "EOL expected, ";
      break;
    default:
      break;
    }
    switch(e.actual)
    {
      case Token::STRING:
      errorMsg += "string received.";
      break;
    case Token::SYMBOL:
      errorMsg += "symbol received.";
      break;
    case Token::NUMBER:
      errorMsg += "number received.";
      break;
    case Token::END:
      errorMsg += "EOL received.";
      break;
    default:
      break;
    }
    cout << "Unknown format. " << errorMsg << endl;
    debugAssert(e.expected != e.actual);
    (void)e;
  }
  
  //cout << endl << "Number of vertices: " << va.size() << endl; 
  //cout << "Number of normals: " << vn.size() << endl;
  //cout << "Number of tex coords: " << vt.size() << endl;

  // Set the vertices, vertex normals, and texture coordinates
  int pFlipped = 0;
  for(int i = 0; i < objects.size(); i++)
  {
    for(int j = 0; j < oVI[i].size(); j++)
    {
      objects[i].geometry.vertexArray.append(_scale * va[oVI[i][j]]);
      objects[i].geometry.normalArray.append(vn[oNI[i][j]]);
      objects[i].texCoordArray.append(vt[oTI[i][j]]);
    }
    
    // Make sure all triangles are facing the right direction
    for(int j = 0; j < objects[i].triangleArray.size()/3; j++)
    {
      Vector3 a = objects[i].geometry.vertexArray[objects[i].triangleArray[j*3]];
      Vector3 b = objects[i].geometry.vertexArray[objects[i].triangleArray[j*3+1]];
      Vector3 c = objects[i].geometry.vertexArray[objects[i].triangleArray[j*3+2]];
      Vector3 midpoint = (a + b + c) / 3;

      Plane p(a, b, c);

      Vector3 normal = (objects[i].geometry.normalArray[objects[i].triangleArray[j*3]] + objects[i].geometry.normalArray[objects[i].triangleArray[j*3+1]] + objects[i].geometry.normalArray[objects[i].triangleArray[j*3+2]]) / 3;
      normal = normal + midpoint;

      if(!p.halfSpaceContains(normal))
      {
	++pFlipped;
	// Swap vertex indices
	int swap = objects[i].triangleArray[j*3+1];
	objects[i].triangleArray[j*3+1] = objects[i].triangleArray[j*3+2];
	objects[i].triangleArray[j*3+2] = swap;
      }
    }
    
    // Set VAR
    VARAreaRef area = the_app.var_static;
    objects[i].vertexArray = VAR(objects[i].geometry.vertexArray, area);
    objects[i].normalArray = VAR(objects[i].geometry.normalArray, area);
    objects[i].texArray    = VAR(objects[i].texCoordArray, area);
  }

  //cout << "Flipped planes: " << pFlipped << endl;
}


StudioModel::Object *StudioModel::getObjectByName(const std::string& name)
{
  if(objectTable.containsKey(name))
  {
    return objectTable.get(name);
  }

  return NULL;
}

void StudioModel::pose(const CoordinateFrame &cframe, Array<PosedStudioModelObjectRef>& posedObjects)
{
	if(posedObjects.size() != objects.size())
	{
		posedObjects.resize(objects.size());
	}

	for(int i = 0; i < objects.size(); i++)
	{
		posedObjects[i] = objects[i].pose(cframe);
	}
}

PosedStudioModelObjectRef StudioModel::poseByName(const std::string& name, const CoordinateFrame &cframe)
{
	if(objectTable.containsKey(name))
  	{
   		StudioModel::Object *obj = objectTable.get(name);
		return obj->pose(cframe);
  	}

	return NULL;
}

void StudioModel::render(RenderDevice *RD, StudioModel::Object *obj)
{
    if(obj->texture.notNull())
    {
      RD->setTexture(0, obj->texture);
      RD->setColor(obj->diffuse);
      RD->setShininess(obj->shininess);
      RD->setAmbientLightColor(obj->ambient);

      RD->beginIndexedPrimitives();
      RD->setVertexArray(obj->vertexArray);
      RD->setNormalArray(obj->normalArray);
      RD->setTexCoordArray(0, obj->texArray);
      RD->sendIndices(RenderDevice::TRIANGLES, obj->triangleArray);
      RD->endIndexedPrimitives();
    } else
    {
      RD->setTexture(0, NULL);
      RD->setColor(obj->diffuse);
      RD->setShininess(obj->shininess);
      RD->setAmbientLightColor(obj->ambient);

      RD->beginIndexedPrimitives();
      RD->setVertexArray(obj->vertexArray);
      RD->setNormalArray(obj->normalArray);
      //RD->setTexCoordArray(objects[obj].texArray);
      RD->sendIndices(RenderDevice::TRIANGLES, obj->triangleArray);
      RD->endIndexedPrimitives();
    }
}

void StudioModel::render(RenderDevice *RD, float alpha) {
  //renderDevice = RD; // IS3D
  //renderDevice->pushState();
  RD->pushState();
  //RD->setColor(Color3::white());
  //for(Lib3dsNode *p = file->nodes; p != 0; p = p->next)
  //{
  //  parseNode(p);
  //}
  
  
  int t, i, j, obj;

  Color4 color = Color3::white();
  color.a = alpha;

  //glEnable(GL_LINE_SMOOTH);
  //RD->setColor(Color3::white());
  //renderDevice->setLineWidth(.5);
  
  //renderDevice->setPolygonOffset(.5);
  //renderDevice->setDepthTest(RenderDevice::DEPTH_LEQUAL);
  // First iteration draws surfaces, 2nd draws wireframe
  if(_twoSided)
  {
    RD->setCullFace(RenderDevice::CULL_NONE);
  }
  for(obj = 0; obj < objects.size(); obj++)
  {
    if(objects[obj].texture.notNull())
    {
      RD->setTexture(0, objects[obj].texture);
      if (fuzzyEq(color.a, 1.f)) {
        RD->setColor(Color3::white());
      } else {
        RD->setColor(color);
      }
      
      RD->setShininess(objects[obj].shininess);
      RD->setSpecularCoefficient(objects[obj].specular);
      RD->setAmbientLightColor(objects[obj].ambient);
#ifndef USE_VAR
      RD->beginPrimitive(RenderDevice::TRIANGLES);
      for (t = 0; t < objects[obj].triangleArray.size(); ++t) 
      {
	RD->setNormal(objects[obj].faceNormalArray[t]);
	for (i = 0; i < 3; ++i) 
	{
	  const int idx = objects[obj].triangleArray[t].index[i];
	  RD->setNormal(objects[obj].geometry.normalArray[idx]);
	  RD->setTexCoord(0, objects[obj].texCoordArray[idx]);
	  RD->sendVertex(objects[obj].geometry.vertexArray[idx]);
	}
      }
      RD->endPrimitive();
#else
      RD->beginIndexedPrimitives();
      RD->setVertexArray(objects[obj].vertexArray);
      RD->setNormalArray(objects[obj].normalArray);
      RD->setTexCoordArray(0, objects[obj].texArray);
      RD->sendIndices(RenderDevice::TRIANGLES, objects[obj].triangleArray);
      RD->endIndexedPrimitives();
#endif
    } else
    {
      RD->setTexture(0, NULL);
      color = objects[obj].diffuse;
      color.a = alpha;
      RD->setColor(color);
      RD->setShininess(objects[obj].shininess);
      //RD->setSpecularCoefficient(objects[obj].specular);
      RD->setAmbientLightColor(objects[obj].ambient);
#ifndef USE_VAR
      RD->beginPrimitive(RenderDevice::TRIANGLES);
      for (t = 0; t < objects[obj].triangleArray.size(); ++t) 
      {
	RD->setNormal(objects[obj].faceNormalArray[t]);
	for (i = 0; i < 3; ++i) 
	{
	  const int idx = objects[obj].triangleArray[t].index[i];
	  RD->setNormal(objects[obj].geometry.normalArray[idx]);
	  RD->sendVertex(objects[obj].geometry.vertexArray[idx]);
	}
      }
      RD->endPrimitive();
#else
      RD->beginIndexedPrimitives();
      RD->setVertexArray(objects[obj].vertexArray);
      RD->setNormalArray(objects[obj].normalArray);
      //RD->setTexCoordArray(objects[obj].texArray);
      RD->sendIndices(RenderDevice::TRIANGLES, objects[obj].triangleArray);
      RD->endIndexedPrimitives();
#endif
    }
  }
  RD->setTexture(0, NULL);
  //Draw::axes(renderDevice);
  //glDisable(GL_LINE_SMOOTH);
  
  RD->popState();
}

void StudioModel::render(RenderDevice *RD, const Color4& color) 
{
  //renderDevice = RD; // IS3D
  //renderDevice->pushState();
  RD->pushState();
  //for(Lib3dsNode *p = file->nodes; p != 0; p = p->next)
  //{
  //  parseNode(p);
  //}

  int t, i, j, obj;

  //glEnable(GL_LINE_SMOOTH);
  //RD->setColor(Color3::white());
  //renderDevice->setLineWidth(.5);
  
  //renderDevice->setPolygonOffset(.5);
  //renderDevice->setDepthTest(RenderDevice::DEPTH_LEQUAL);
  // First iteration draws surfaces, 2nd draws wireframe
  if(_twoSided)
  {
    RD->setCullFace(RenderDevice::CULL_NONE);
  }
  for(obj = 0; obj < objects.size(); obj++)
  {
    if(objects[obj].texture.notNull())
    {
      RD->setTexture(0, objects[obj].texture);
      RD->setColor(color);
      RD->setShininess(objects[obj].shininess);
      RD->setAmbientLightColor(objects[obj].ambient);
#ifndef USE_VAR
      RD->beginPrimitive(RenderDevice::TRIANGLES);
      for (t = 0; t < objects[obj].triangleArray.size(); ++t) 
      {
	RD->setNormal(objects[obj].faceNormalArray[t]);
	for (i = 0; i < 3; ++i) 
	{
	  const int idx = objects[obj].triangleArray[t].index[i];
	  RD->setNormal(objects[obj].geometry.normalArray[idx]);
	  RD->setTexCoord(0, objects[obj].texCoordArray[idx]);
	  RD->sendVertex(objects[obj].geometry.vertexArray[idx]);
	}
      }
      RD->endPrimitive();
#else
      RD->beginIndexedPrimitives();
      RD->setVertexArray(objects[obj].vertexArray);
      RD->setNormalArray(objects[obj].normalArray);
      RD->setTexCoordArray(0, objects[obj].texArray);
      RD->sendIndices(RenderDevice::TRIANGLES, objects[obj].triangleArray);
      RD->endIndexedPrimitives();
#endif
    } else
    {
      RD->setTexture(0, NULL);
      Color4 temp = objects[obj].diffuse;
      temp.a = color.a;
      RD->setColor(temp);
      RD->setShininess(objects[obj].shininess);
      RD->setAmbientLightColor(objects[obj].ambient);
#ifndef USE_VAR
      RD->beginPrimitive(RenderDevice::TRIANGLES);
      for (t = 0; t < objects[obj].triangleArray.size(); ++t) 
      {
	RD->setNormal(objects[obj].faceNormalArray[t]);
	for (i = 0; i < 3; ++i) 
	{
	  const int idx = objects[obj].triangleArray[t].index[i];
	  RD->setNormal(objects[obj].geometry.normalArray[idx]);
	  RD->sendVertex(objects[obj].geometry.vertexArray[idx]);
	}
      }
      RD->endPrimitive();
#else
      RD->beginIndexedPrimitives();
      RD->setVertexArray(objects[obj].vertexArray);
      RD->setNormalArray(objects[obj].normalArray);
      //RD->setTexCoordArray(objects[obj].texArray);
      RD->sendIndices(RenderDevice::TRIANGLES, objects[obj].triangleArray);
      RD->endIndexedPrimitives();
#endif
    }
  }
  RD->setTexture(0, NULL);
  //Draw::axes(renderDevice);
  //glDisable(GL_LINE_SMOOTH);
  
  RD->popState();
}

PosedStudioModelObjectRef StudioModel::Object::pose(const CoordinateFrame& cframe)
{
	return new PosedStudioModelObject(this, cframe, GMaterial(), false);
}

PosedStudioModelObject::PosedStudioModelObject(StudioModel::Object* objectRef, const CoordinateFrame& pframe, const GMaterial& mat, bool useMat):
_objectRef(objectRef), _cframe(pframe), _material(mat), _useMat(useMat)
{
}

string PosedStudioModelObject::name() const
{
	return _objectRef->name;
}

void PosedStudioModelObject::getCoordinateFrame(CoordinateFrame& c) const
{
	c = _cframe;
}

const MeshAlg::Geometry& PosedStudioModelObject::objectSpaceGeometry() const
{
	return _objectRef->geometry;
}

const Array<MeshAlg::Face>& PosedStudioModelObject::faces() const
{
	return _objectRef->faces;
}

const Array<MeshAlg::Edge>& PosedStudioModelObject::edges() const
{
	return _objectRef->edges;
}

const Array<MeshAlg::Vertex>& PosedStudioModelObject::vertices() const
{
	return _objectRef->vertices;
}

const Array<MeshAlg::Face>& PosedStudioModelObject::weldedFaces() const
{
	return _objectRef->weldedFaces;
}

const Array<MeshAlg::Edge>& PosedStudioModelObject::weldedEdges() const
{
	return _objectRef->weldedEdges;
}

const Array<MeshAlg::Vertex>& PosedStudioModelObject::weldedVertices() const
{
	return _objectRef->weldedVertices;
}

const Array<int>& PosedStudioModelObject::triangleIndices() const
{
	return _objectRef->triangleArray;
}

const Array<Vector3>& PosedStudioModelObject::objectSpaceFaceNormals(bool normalize) const
{
	return _objectRef->faceNormalArray;
}

void PosedStudioModelObject::getObjectSpaceBoundingSphere(Sphere& _sphere) const
{
	_sphere = _objectRef->bsphere;
}

void PosedStudioModelObject::getObjectSpaceBoundingBox(Box& _box) const
{
	_box = _objectRef->bbox;
}

void PosedStudioModelObject::render(RenderDevice* renderDevice) const
{
	//do more here?
	renderDevice->pushState();
        CoordinateFrame objToWorld = renderDevice->getObjectToWorldMatrix();
	renderDevice->setObjectToWorldMatrix(objToWorld*_cframe);
	StudioModel::render(renderDevice, _objectRef);
	renderDevice->popState();
}

int PosedStudioModelObject::numBoundaryEdges() const
{
	return _objectRef->numBoundaryEdges;
}

int PosedStudioModelObject::numWeldedBoundaryEdges() const
{
	return _objectRef->numWeldedBoundaryEdges;
}

/*
void XIFSModel::save(const std::string& filename) {

    Array<int> index;
    for (int i = 0; i < triangleArray.size(); ++i) {
        index.append(triangleArray[i].index[0], triangleArray[i].index[1], triangleArray[i].index[2]);
    }

    IFSModel::save(filename, name, index, geometry.vertexArray, texCoordArray);
}
*/
