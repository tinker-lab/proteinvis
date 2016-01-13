#include "CFrameMgr.H"

CFrameMgr::CFrameMgr()
{
	_roomToVirtual = glm::dmat4(1.0);
}

CFrameMgr::~CFrameMgr()
{
}

glm::dmat4 CFrameMgr::getRoomToVirtualSpaceFrame()
{
	return _roomToVirtual;
}

void CFrameMgr::setRoomToVirtualSpaceFrame(const glm::dmat4 &roomToVirtual)
{
	_roomToVirtual = roomToVirtual;
}

void CFrameMgr::setRoomToVirtualScaleFactor(double scale)
{
	_roomToVirtualScale = scale;
}

glm::dvec3 CFrameMgr::roomPointToVirtualSpace(const glm::dvec3 &v)
{
	glm::dvec3 v2(_roomToVirtual[0][0] * v[0] + _roomToVirtual[1][0] * v[1] + _roomToVirtual[2][0] * v[2] + _roomToVirtual[3][0],
				 _roomToVirtual[0][1] * v[0] + _roomToVirtual[1][1] * v[1] + _roomToVirtual[2][1] * v[2] + _roomToVirtual[3][1],
				 _roomToVirtual[0][2] * v[0] + _roomToVirtual[1][2] * v[1] + _roomToVirtual[2][2] * v[2] + _roomToVirtual[3][2]);
		
	return v2;
}

glm::dvec3 CFrameMgr::roomVectorToVirtualSpace(const glm::dvec3 &v)
{
	glm::dvec3 v2(_roomToVirtual[0][0] * v[0] + _roomToVirtual[1][0] * v[1] + _roomToVirtual[2][0] * v[2],
				 _roomToVirtual[0][1] * v[0] + _roomToVirtual[1][1] * v[1] + _roomToVirtual[2][1] * v[2],
				 _roomToVirtual[0][2] * v[0] + _roomToVirtual[1][2] * v[1] + _roomToVirtual[2][2] * v[2]);
	return v2;
}

glm::dvec3 CFrameMgr::roomNormalToVirtualSpace(const glm::dvec3 &v)
{
	glm::dvec3 v2(_roomToVirtual[0][0] * v[0] + _roomToVirtual[1][0] * v[1] + _roomToVirtual[2][0] * v[2],
				 _roomToVirtual[0][1] * v[0] + _roomToVirtual[1][1] * v[1] + _roomToVirtual[2][1] * v[2],
				 _roomToVirtual[0][2] * v[0] + _roomToVirtual[1][2] * v[1] + _roomToVirtual[2][2] * v[2]);
	return v2;
}

double CFrameMgr::roomDistanceToVirtualSpace(const double &d)
{
	return glm::length(roomVectorToVirtualSpace(glm::dvec3(d,0,0)));
}


glm::dvec3 CFrameMgr::virtualPointToRoomSpace(const glm::dvec3 &v)
{
	/*
	double p[3];
    p[0] = v[0] - _roomToVirtual[3][0];
    p[1] = v[1] - _roomToVirtual[3][1];
    p[2] = v[2] - _roomToVirtual[3][2];
    glm::dvec3 v2(_roomToVirtual[0][0] * p[0] + _roomToVirtual[0][1] * p[1] + _roomToVirtual[0][2] * p[2],
				_roomToVirtual[1][0] * p[0] + _roomToVirtual[1][1] * p[1] + _roomToVirtual[1][2] * p[2],
				_roomToVirtual[2][0] * p[0] + _roomToVirtual[2][1] * p[1] + _roomToVirtual[2][2] * p[2]);
	*/
	glm::dvec3 v2 = pointMultiply(glm::inverse(_roomToVirtual), v);

	return v2;
}

glm::dvec3 CFrameMgr::virtualVectorToRoomSpace(const glm::dvec3 &v)
{
	glm::dmat4 trans = glm::inverse(_roomToVirtual);
	glm::dvec3 v2(trans[0][0] * v[0] + trans[1][0] * v[1] + trans[2][0] * v[2],
				 trans[0][1] * v[0] + trans[1][1] * v[1] + trans[2][1] * v[2],
				 trans[0][2] * v[0] + trans[1][2] * v[1] + trans[2][2] * v[2]);

	return v2;
}

glm::dvec3 CFrameMgr::virtualNormalToRoomSpace(const glm::dvec3 &v)
{
	glm::dmat4 trans = glm::inverse(_roomToVirtual);
	glm::dvec3 v2(trans[0][0] * v[0] + trans[1][0] * v[1] + trans[2][0] * v[2],
				 trans[0][1] * v[0] + trans[1][1] * v[1] + trans[2][1] * v[2],
				 trans[0][2] * v[0] + trans[1][2] * v[1] + trans[2][2] * v[2]);
	return v2;
}

double CFrameMgr::virtualDistanceToRoomSpace(const double &d)
{
	return glm::length(virtualVectorToRoomSpace(glm::dvec3(d,0,0)));
}

glm::dmat4 CFrameMgr::roomToVirtualSpace(const glm::dmat4 &c)
{
	glm::dmat4 cv = _roomToVirtual * c;
	return cv;
}

glm::dmat4 CFrameMgr::virtualToRoomSpace(const glm::dmat4 &c)
{
	glm::dmat4 cr = glm::inverse(_roomToVirtual) * c;
	return cr;
}

glm::dmat4 CFrameMgr::getVirtualToRoomSpaceFrame()
{
	// Compute virtualToRoom transformation
	return glm::inverse(_roomToVirtual);
}

glm::dmat4 CFrameMgr::getVirtualToRoomSpaceFrameNoScale()
{
	glm::dmat4 roomToVirtualNoScale = _roomToVirtual;
	roomToVirtualNoScale[0][0] /= _roomToVirtualScale;
	roomToVirtualNoScale[1][1] /= _roomToVirtualScale;
	roomToVirtualNoScale[2][2] /= _roomToVirtualScale;
	roomToVirtualNoScale[3][0] /= _roomToVirtualScale;
	roomToVirtualNoScale[3][0] /= _roomToVirtualScale;
	roomToVirtualNoScale[3][0] /= _roomToVirtualScale;

	return glm::inverse(roomToVirtualNoScale);
}

glm::dvec3 CFrameMgr::convertPointToObjectSpace(const glm::dmat4 &frame, const glm::dvec3 &v)
{
	/*
	double p[3];
    p[0] = v[0] - frame[3][0];
    p[1] = v[1] - frame[3][1];
    p[2] = v[2] - frame[3][2];
    return glm::dvec3(frame[0][0] * p[0] + frame[0][1] * p[1] + frame[0][2] * p[2],
					 frame[1][0] * p[0] + frame[1][1] * p[1] + frame[1][2] * p[2],
					 frame[2][0] * p[0] + frame[2][1] * p[1] + frame[2][2] * p[2]);
	*/

	glm::dvec3 v2 = pointMultiply(glm::inverse(frame), v);
	return v2;
}

glm::dvec3 CFrameMgr::convertPointToWorldSpace(const glm::dmat4 &frame, const glm::dvec3 &v)
{
	return glm::dvec3(frame[0][0] * v[0] + frame[1][0] * v[1] + frame[2][0] * v[2] + frame[3][0],
					 frame[0][1] * v[0] + frame[1][1] * v[1] + frame[2][1] * v[2] + frame[3][1],
					 frame[0][2] * v[0] + frame[1][2] * v[1] + frame[2][2] * v[2] + frame[3][2]);
}

glm::dvec3 CFrameMgr::convertVectorToObjectSpace(const glm::dmat4 &frame, const glm::dvec3 &v)
{
	 return v * glm::dmat3(frame);
}

glm::dvec3 CFrameMgr::convertVectorToWorldSpace(const glm::dmat4 &frame, const glm::dvec3 &v)
{
	return glm::dmat3(frame) * v;
}

glm::vec3 CFrameMgr::vectorMultiply(const glm::mat4 &frame, const glm::vec3 &v)
{
	return glm::vec3(frame[0][0] * v[0] + frame[1][0] * v[1] + frame[2][0] * v[2],
				 frame[0][1] * v[0] + frame[1][1] * v[1] + frame[2][1] * v[2],
				 frame[0][2] * v[0] + frame[1][2] * v[1] + frame[2][2] * v[2]);
}
	
glm::vec3 CFrameMgr::pointMultiply(const glm::mat4 &frame, const glm::vec3 &pt)
{
	return glm::vec3(frame[0][0] * pt[0] + frame[1][0] * pt[1] + frame[2][0] * pt[2] + frame[3][0],
				 frame[0][1] * pt[0] + frame[1][1] * pt[1] + frame[2][1] * pt[2] + frame[3][1],
				 frame[0][2] * pt[0] + frame[1][2] * pt[1] + frame[2][2] * pt[2] + frame[3][2]);
}

glm::dvec3 CFrameMgr::vectorMultiply(const glm::dmat4 &frame, const glm::dvec3 &v)
{
	return glm::dvec3(frame[0][0] * v[0] + frame[1][0] * v[1] + frame[2][0] * v[2],
				 frame[0][1] * v[0] + frame[1][1] * v[1] + frame[2][1] * v[2],
				 frame[0][2] * v[0] + frame[1][2] * v[1] + frame[2][2] * v[2]);
}
	
glm::dvec3 CFrameMgr::pointMultiply(const glm::dmat4 &frame, const glm::dvec3 &pt)
{
	return glm::dvec3(frame[0][0] * pt[0] + frame[1][0] * pt[1] + frame[2][0] * pt[2] + frame[3][0],
				 frame[0][1] * pt[0] + frame[1][1] * pt[1] + frame[2][1] * pt[2] + frame[3][1],
				 frame[0][2] * pt[0] + frame[1][2] * pt[1] + frame[2][2] * pt[2] + frame[3][2]);
}