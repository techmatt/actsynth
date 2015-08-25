
#include "libsynth.h"

string Model::imageURL() const
{
    return "file:///V:/data/models/repositories/g3dw/image/" + ml::util::remove(id, "wss.") + ".jpg";
}

string Model::imageHTML(unsigned int width, unsigned int height) const
{
    return "<img src=\"" + imageURL() + "\" width=" + to_string(width) + " height=" + to_string(height) + "/>";
}
