/**
 * /author Bret Jackson
 *
 * /file  AABox.H
 * /brief axis aligned bounding box based on G3D's class
 *
 */ 

#ifndef AABOX_H
#define AABOX_H

#include <glm/glm.hpp>
#include <memory>

typedef std::shared_ptr<class AABox> AABoxRef;

class AABox : public std::enable_shared_from_this<AABox>
{
public:
	AABox(const AABox &box);
	AABox(const glm::dvec3 &low, const glm::dvec3 &high);
	AABox(const glm::dvec3 point); // create a zero volume box around a point
	~AABox();
	void set(const glm::dvec3 &low, const glm::dvec3 &high);

	glm::dvec3 center() const;
	glm::dvec3 corner(int index) const;

	glm::dvec3 high() const;
	glm::dvec3 low() const;

    /**
     Distance from corner(0) to the next corner along axis a.
     */
	float extent(int a) const;
	glm::dvec3 extent() const;

	bool contains(const glm::dvec3 &point) const;
	bool contains(const AABox &other) const;

	//Sets this to the smallest AABox that encapsulates both. 
	void merge(const AABox &other);
	void merge(const glm::dvec3 &point);

	/** Returns true if there is any overlap */
    bool intersects(const AABox& other) const;

	bool operator==(const AABox& other) const;
	bool operator!=(const AABox& other) const;
	
private:
	glm::dvec3 _low;
	glm::dvec3 _high;
};
#endif