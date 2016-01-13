/**
* /author Bret Jackson
*
* /file  TextureMgr.H
* /brief Keeps track of a database of textures to load based on the config file
* 
* Note: this pretty much came from gfxmgr in vrbase
*/ 

#ifndef TEXTUREMGR_H
#define TEXTUREMGR_H

#include <MVRCore/StringUtils.H>
#include <MVRCore/ConfigVal.H>
#include <vector>
#include <map>
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include "Texture.h"

typedef std::shared_ptr<class TextureMgr> TextureMgrRef;

/*
Description of Texture Handling:

The class holds a database of textures accessable by name.  Any texture files
specified in the ConfigVal LoadTextures will be automatically
loaded.  Textures in LoadTextures are separated by a semicolon.
The format for specifying a texture is:

\verbatim    
<filename> <TexMgr keyname> [wrap mode] [min interpolate mode] [mag interpolate mode;
\endverbatim

Cubemaps may be loaded by including the wildcard *. This will substutute +x, -x, +y, etc for the wildcard. For example:
\verbatim
LoadTextures sky*.jpg envmap;
\endverbatim

Arguments in <> are required, args in [] are optional.  For 
example, in a config file you could have:

\verbatim
LoadTextures   file1.jpg                  background-image GL_REPEAT GL_LINEAR_MIPMAP_LINEAR GL_LINEAR; \
file2.jpg                  foreground-image ; \
file3.jpg  myTextureWithAnAlpha ;
\endverbatim

*/
class TextureMgr : public std::enable_shared_from_this<TextureMgr>
{
public:
	TextureMgr();
	~TextureMgr();

	//TODO: currently assumes all config val textures are 2D
	void							loadTexturesFromConfigVal(int threadId, const std::string &configName);
	std::shared_ptr<Texture>		getTexture(int threadId, const std::string &keyName);
	std::vector<std::shared_ptr<Texture> >	getTextures(int threadId, const std::vector<std::string> &keyNames);
	void							setTextureEntry(int threadId, const std::string &keyName, std::shared_ptr<Texture> tex);
	void							removeTextureEntry(int threadId, const std::string &keyName);
	std::string						lookupTextureKeyName(int threadId, std::shared_ptr<Texture> tex);
	std::vector<std::string>		getAllTextureKeys(int threadId);
	static void						generateCubeMapFilenames(const std::string& src, std::string realFilename[6]);
	
private:
	std::string								 _configName;
	std::vector< std::map< std::string, std::shared_ptr<Texture> > > _texMap;
	boost::mutex _configMutex;
};

#endif