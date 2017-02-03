#ifndef INCLUDE_EDK3_GEOMETRY_H_
#define INCLUDE_EDK3_GEOMETRY_H_ 1

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Jose L. Hidalgo 
// Geometry Class.
// ----------------------------------------------------------------------------

#include "referenced.h"
#include "ref_ptr.h"
#include "scoped_array.h"
#include "constants.h"

namespace EDK3 {
  
  class Material;

  class Geometry : public Referenced {
  public:
    virtual bool bindAttribute(
          const Attribute a,
          unsigned int where_to_bind_attribute) const = 0;
    virtual void render() const = 0;

	// Draws the geometry (binding the attributes needed first) for the given material
	// which must be the current (enabled) material in use.
    void bindAttributesAndRenderWithCurrentMaterial(const Material *m) const;

    void set_name(const char *name) { name_.copy(name, name+strlen(name)+1); }
    const char* name() const { return name_.get(); }
  protected:
    Geometry() {}
    virtual ~Geometry() {}

  private:
    scoped_array<char> name_;
    Geometry(const Geometry&);
    Geometry& operator=(const Geometry &);
  };

  void CreateCube(
      ref_ptr<Geometry> *output,
      const float cube_size = 1.0f,
      const bool build_normals = true,
      const bool build_tex_coords = true);

  void CreateQuad(
      ref_ptr<Geometry> *output, 
      const float width = 1.0f,
      const float height = 1.0f,
      const bool build_normals = true,
      const bool build_tex_coords = true);

  bool LoadObj(
      const char *filename,
      scoped_array<ref_ptr<Geometry> > *geometries,
      scoped_array<char> *error_log );

} /* end of EDK Namespace */

#endif
