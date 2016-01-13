#include "TextureMgr.h"

TextureMgr::TextureMgr()
{
}

TextureMgr::~TextureMgr()
{
}

std::shared_ptr<Texture> TextureMgr::getTexture(int threadId, const std::string &keyName)
{
	assert(threadId < _texMap.size());
	if (_texMap[threadId].find(keyName) != _texMap[threadId].end())
		return _texMap[threadId][keyName];
	else
		return nullptr;
}

std::vector<std::shared_ptr<Texture> > TextureMgr::getTextures(int threadId, const std::vector<std::string> &keyNames)
{
	assert(threadId < _texMap.size());
	std::vector<std::shared_ptr<Texture> > ta;
	for (int i=0;i<keyNames.size();i++) {
		ta.push_back(getTexture(threadId, keyNames[i]));
	}
	return ta;
}

void TextureMgr::setTextureEntry(int threadId, const std::string &keyName, std::shared_ptr<Texture> tex)
{
	_configMutex.lock();
	if (threadId >= _texMap.size()) {
		for(int i=_texMap.size(); i < threadId + 1; i++) {
			std::map<std::string, std::shared_ptr<Texture> > threadMap;
			_texMap.push_back(threadMap);
		}
	}
	_configMutex.unlock();

	_texMap[threadId].insert(std::pair<std::string, std::shared_ptr<Texture> >(keyName, tex));
}

void TextureMgr::removeTextureEntry(int threadId, const std::string &keyName)
{
	assert(threadId < _texMap.size());
	_texMap[threadId].erase(keyName);
}


std::string TextureMgr::lookupTextureKeyName(int threadId, std::shared_ptr<Texture> tex)
{
	assert(threadId < _texMap.size());
	for(auto it = _texMap[threadId].begin(); it != _texMap[threadId].end(); ++it) {
		if (it->second == tex) {
			return it->first;
		}
	}
	return std::string("");
}

std::vector<std::string> TextureMgr::getAllTextureKeys(int threadId)
{
	assert(threadId < _texMap.size());
	std::vector<std::string> keys;
	for(auto it = _texMap[threadId].begin(); it != _texMap[threadId].end(); ++it) {
		keys.push_back(it->first);
	}
	return keys;
}

void TextureMgr::loadTexturesFromConfigVal(int threadId, const std::string &configName)
{
	_configMutex.lock();

	if (threadId >= _texMap.size()) {
		for(int i=_texMap.size(); i < threadId + 1; i++) {
			std::map<std::string, std::shared_ptr<Texture> > threadMap;
			_texMap.push_back(threadMap);
		}
	}

	std::string in = MinVR::ConfigVal(configName,"");

	while (in.size()) {
		std::string               filename;
		std::string               keyname;
		std::string               token;
		GLenum					  wrapMode = GL_REPEAT;
		GLenum					  minInterpMode = GL_LINEAR_MIPMAP_LINEAR;
		GLenum					  magInterpMode = GL_LINEAR;

		int semicolonpos = in.find(";");
		BOOST_ASSERT_MSG(semicolonpos >= 0, "Expected a ; to signal the end of each texture in the LoadTextures ConfigVal.");

		int commapos = in.find(",");
		//bool hasalpha = ((commapos >= 0) && (commapos <= semicolonpos));

		if (!MinVR::popNextToken(in, filename, true))
			BOOST_ASSERT_MSG(false, "Expected filename while loading texture from ConfigVal");

		/*
		if (hasalpha) {
			// remove the , separating the filename from the alphafilename
			in = in.substr(1);
			if (!MinVR::popNextToken(in, alphafilename, true))
				alwaysAssertM(false, "TexMgr expected filename for alpha channel while loading texture from ConfigVal");
			// assume format of RGBA8 if alpha is specified, AUTO doesn't seem to work by default.
			format = std::string("RGBA8");
		}
		*/

		if (!MinVR::popNextToken(in, keyname, true)) {
			BOOST_ASSERT_MSG(false, "TexMgr expected keyname while loading texture from ConfigVal");
		}
		else {

			// Optional wrap mode
			if (MinVR::popNextToken(in, token, true)) {
				if (token == "GL_CLAMP_TO_EDGE") {
					wrapMode = GL_CLAMP_TO_EDGE;
				}
				else if (token == "GL_CLAMP_TO_BORDER") {
					wrapMode = GL_CLAMP_TO_BORDER;
				}
				else if (token == "GL_REPEAT") {
					wrapMode = GL_REPEAT;
				}
				else if (token == "GL_MIRRORED_REPEAT") {
					wrapMode = GL_MIRRORED_REPEAT;
				}
				else {
					BOOST_ASSERT_MSG(false, "Unsupported wrap mode");
				}
			}

			// optional interpolation mode
			if (MinVR::popNextToken(in, token, true)) {
				if (token == "GL_NEAREST") {
					minInterpMode = GL_NEAREST;
				}
				else if (token == "GL_LINEAR") {
					minInterpMode = GL_LINEAR;
				}
				else if (token == "GL_LINEAR_MIPMAP_LINEAR") {
					minInterpMode = GL_LINEAR_MIPMAP_LINEAR;
				}
				else {
					BOOST_ASSERT_MSG(false, "Unsupported min interpolation mode");
				}
			}

			// optional interpolation mode
			if (MinVR::popNextToken(in, token, true)) {
				if (token == "GL_NEAREST") {
					magInterpMode = GL_NEAREST;
				}
				else if (token == "GL_LINEAR") {
					magInterpMode = GL_LINEAR;
				}
				else {
					BOOST_ASSERT_MSG(false, "Unsupported mag interpolation mode");
				}
			}

			// Remove the semicolon
			in = in.substr(1);
		}

		filename = MinVR::decygifyPath(MinVR::replaceEnvVars(filename));

		if (filename.find("*") != std::string::npos) {
			// cubemap
			std::string filenames[6];
			generateCubeMapFilenames(filename, filenames);

			for(int i=0; i < 6; i++) {
				if (!boost::filesystem::exists(filenames[i])) {
					BOOST_ASSERT_MSG(false, ("Texture file does not exist: " + filenames[i]).c_str());
				}
			}
			
			std::string msg;
			msg = "Loading cubemap texture " + filename + " with keyname " + keyname;
			std::cout << msg << std::endl;

			std::shared_ptr<Texture> tex = Texture::createCubeMapFromFiles(filenames, true, 4);
			tex->setTexParameteri(GL_TEXTURE_WRAP_S, wrapMode);
			tex->setTexParameteri(GL_TEXTURE_WRAP_T, wrapMode);
			tex->setTexParameteri(GL_TEXTURE_WRAP_R, wrapMode);
			tex->setTexParameteri(GL_TEXTURE_MIN_FILTER, minInterpMode);
			tex->setTexParameteri(GL_TEXTURE_MAG_FILTER, magInterpMode);
			tex->setFileName(filename);
			_texMap[threadId].insert(std::pair<std::string, std::shared_ptr<Texture> >(keyname, tex));
		}
		else {
			//alphafilename = MinVR::decygifyPath(MinVR::replaceEnvVars(alphafilename));

			if (!boost::filesystem::exists(filename)) {
				BOOST_ASSERT_MSG(false, ("Texture file does not exist: " + filename).c_str());
			}
			//if ((hasalpha) && (!FileSystem::exists(alphafilename)))
			//	alwaysAssertM(false, "Texture file does not exist: " + alphafilename);

			std::string msg;
			//if (hasalpha) {
			//	msg = "Loading texture " + filename + " with alpha texture " + alphafilename + " and keyname " + keyname;
			//}
			//else {
				msg = "Loading texture " + filename + " with keyname " + keyname;
			//}
			std::cout << msg << std::endl;

			std::shared_ptr<Texture> tex = Texture::create2DTextureFromFile(filename, true, 4);
			tex->setTexParameteri(GL_TEXTURE_WRAP_S, wrapMode);
			tex->setTexParameteri(GL_TEXTURE_WRAP_T, wrapMode);
			tex->setTexParameteri(GL_TEXTURE_WRAP_R, wrapMode);
			tex->setTexParameteri(GL_TEXTURE_MIN_FILTER, minInterpMode);
			tex->setTexParameteri(GL_TEXTURE_MAG_FILTER, magInterpMode);
			tex->setTexParameterf(GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
			tex->setFileName(filename);
			_texMap[threadId].insert(std::pair<std::string, std::shared_ptr<Texture> >(keyname, tex));
		}
	}
	_configMutex.unlock();
}

void TextureMgr::generateCubeMapFilenames(const std::string& src, std::string realFilename[6])
{
    std::string filenameBase, filenameExt;
	const std::string splitter = "*";
	size_t i = src.rfind(splitter);
    if (i != std::string::npos) {
        filenameBase = src.substr(0, i);
        filenameExt  = src.substr(i + 1, src.size() - i - splitter.length()); 
    }

	std::string face[6] = {"+x", "-x", "+y", "-y", "+z", "-z"};
    for (int f = 0; f < 6; ++f) {
        realFilename[f] = filenameBase + face[f] + filenameExt;
    }
}