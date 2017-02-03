#ifndef INCLUDE_EDK3_MATERIAL_H_
#define INCLUDE_EDK3_MATERIAL_H_ 1

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Jose L. Hidalgo 
// Material Class.
// ----------------------------------------------------------------------------

#include "referenced.h"
#include "constants.h"

namespace EDK3 {

  class MaterialSettings;
  class Drawable;

  class Material : public virtual Referenced {
  public:
    // Returns true if the material can be set with the given material
    // settings
    virtual bool enable(const MaterialSettings *) const = 0;
    // called after enable to setup the camera matrix
    virtual void setupCamera(const float projecton[16], const float view[16]) const = 0;
    // called after enable to setup the model matrix
    // model is the transform from local->world coordinates
    virtual void setupModel(const float model[16]) const = 0;
    
    virtual unsigned int num_attributes_required() const = 0;
    virtual Attribute attribute_at_index(
        const unsigned int attrib_idx) const = 0;
    virtual Type attribute_type_at_index(
        const unsigned int attrib_index) const = 0;

    // draw a FullScreen Quad with the given material/material-settings
    void drawFullScreenQuad(const MaterialSettings *ms) const;

    // draws a portion of the screen with the given material/material-settings
    // min_x, min_y, max_x, max-y are relative to current FB/Window, in the range
    // from 0.0 to 1.0 each.
    void drawRelativeToScreenQuad(
        float min_x,
        float min_y,
        float max_x,
        float max_y,
        const MaterialSettings *mat) const;

    // draws a portion of the screen with the given material/material-settings
    // position is given in pixels
    void drawScreenQuad(
        float min_x,
        float min_y,
        float max_x,
        float max_y,
        const MaterialSettings *mat) const;

  protected:
    Material() {}
    virtual ~Material() {}
  private:
    Material(const Material&);
    Material& operator=(const Material &);
  };

} /* end of EDK Namespace */

#endif
