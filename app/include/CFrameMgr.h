/**
* /author Bret Jackson
*
* /file  CFrameMgr.H
* /brief Keeps track of the room to virtual space transform
* 
* Note: this pretty much came from gfxmgr in vrbase
*/ 

#ifndef CFRAMEMGR_H
#define CFRAMEMGR_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

typedef std::shared_ptr<class CFrameMgr> CFrameMgrRef;

class CFrameMgr : public std::enable_shared_from_this<CFrameMgr>
{
public:
	CFrameMgr();
	~CFrameMgr();

	/** This is the transformation between RoomSpace and VirtualSpace.
	Initially, the two spaces are the same.  As the user navigates
	around VirtualSpace changes, but RoomSpace stays fixed to the
	physical Cave, monitor, screen, trackers, etc..  This
	transformation is composed of two parts: 1. a uniform scale
	factor, 2. a rigid-body CoordinateFrame transformation.
	*/
	glm::dmat4 getRoomToVirtualSpaceFrame();
	glm::dmat4 getVirtualToRoomSpaceFrame();

	glm::dmat4 getVirtualToRoomSpaceFrameNoScale();


	/// Sets the rigid-body CoordinateFrame transformation between the
	/// Room and Virtual spaces.
	void setRoomToVirtualSpaceFrame(const glm::dmat4 &roomToVirtual);

	void setRoomToVirtualScaleFactor(double scale);

	/// Use these to convert from RoomSpace to VirtualSpace, applies a
	/// CoordinateFrame transformation as well as a scale.
	glm::dvec3 roomPointToVirtualSpace(const glm::dvec3 &v);
	glm::dvec3 roomVectorToVirtualSpace(const glm::dvec3 &v);
	glm::dvec3 roomNormalToVirtualSpace(const glm::dvec3 &v);
	double  roomDistanceToVirtualSpace(const double &d);

	/// Use these to convert from VirtualSpace to RoomSpace, applies a
	/// CoordinateFrame transformation as well as a scale.
	glm::dvec3 virtualPointToRoomSpace(const glm::dvec3 &v);
	glm::dvec3 virtualVectorToRoomSpace(const glm::dvec3 &v);
	glm::dvec3 virtualNormalToRoomSpace(const glm::dvec3 &v);
	double  virtualDistanceToRoomSpace(const double &d);

	glm::dmat4 roomToVirtualSpace(const glm::dmat4 &c);
	glm::dmat4 virtualToRoomSpace(const glm::dmat4 &c);

	//Transforms the point into frame's object space.  Assumes that the rotation matrix is orthonormal.
	glm::dvec3 convertPointToObjectSpace(const glm::dmat4 &frame, const glm::dvec3 &v);
	glm::dvec3 convertPointToWorldSpace(const glm::dmat4 &frame, const glm::dvec3 &v);

	//Transforms the vector into frame's object space.  Assumes that the rotation matrix is orthonormal.
	glm::dvec3 convertVectorToObjectSpace(const glm::dmat4 &frame, const glm::dvec3 &v);
	glm::dvec3 convertVectorToWorldSpace(const glm::dmat4 &frame, const glm::dvec3 &v);

	static glm::vec3 vectorMultiply(const glm::mat4 &frame, const glm::vec3 &v);
	static glm::vec3 pointMultiply(const glm::mat4 &frame, const glm::vec3 &pt);

	static glm::dvec3 vectorMultiply(const glm::dmat4 &frame, const glm::dvec3 &v);
	static glm::dvec3 pointMultiply(const glm::dmat4 &frame, const glm::dvec3 &v);

protected:

	/// Transformation from RoomSpace to VirtualSpace
	glm::dmat4 _roomToVirtual;

	double _roomToVirtualScale;
};

#endif