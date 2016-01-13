#include "AABox.H"

AABox::AABox(const AABox &box)
{
	set(box.low(), box.high());
}


AABox::AABox(const glm::dvec3 &low, const glm::dvec3 &high)
{
	set(low, high);
}

AABox::AABox(const glm::dvec3 point)
{
	set(point, point);
}

AABox::~AABox()
{
}

void AABox::set(const glm::dvec3 &low, const glm::dvec3 &high)
{
	_low = low;
	_high = high;
}

glm::dvec3 AABox::center() const
{
	return (_low + _high) * 0.5;
}

glm::dvec3 AABox::high() const
{
	return _high;
}
	
glm::dvec3 AABox::low() const
{
	return _low;
}

glm::dvec3 AABox::corner(int index) const
{
	// default constructor inits all components to 0
    glm::dvec3 v;

    switch (index)
    {
    case 0:
        v.x = _low.x;
        v.y = _low.y;
        v.z = _high.z;
        break;

    case 1:
        v.x = _high.x;
        v.y = _low.y;
        v.z = _high.z;
        break;

    case 2:
        v.x = _high.x;
        v.y = _high.y;
        v.z = _high.z;
        break;

    case 3:
        v.x = _low.x;
        v.y = _high.y;
        v.z = _high.z;
        break;

    case 4:
        v.x = _low.x;
        v.y = _low.y;
        v.z = _low.z;
        break;

    case 5:
        v.x = _high.x;
        v.y = _low.y;
        v.z = _low.z;
        break;

    case 6:
        v.x = _high.x;
        v.y = _high.y;
        v.z = _low.z;
        break;

    case 7:
        v.x = _low.x;
        v.y = _high.y;
        v.z = _low.z;
        break;

    default:
		//Invalid corner index
        assert(false);
        break;
    }

    return v;
}

float AABox::extent(int a) const
{
	assert(a < 3);
	return _high[a] - _low[a];
}

glm::dvec3 AABox::extent() const
{
	return _high - _low;
}

bool AABox::contains(const glm::dvec3 &point) const
{
	return  (point.x >= low().x) &&
            (point.y >= low().y) &&
            (point.z >= low().z) &&
            (point.x <= high().x) &&
            (point.y <= high().y) &&
            (point.z <= high().z);
}

bool AABox::contains(const AABox &other) const
{
	return  (other.high().x <= high().x) &&
            (other.high().y <= high().y) &&
            (other.high().z <= high().z) &&
            (other.low().x >= low().x) &&
            (other.low().y >= low().y) &&
            (other.low().z >= low().z);
}

void AABox::merge(const AABox &other)
{
	set(glm::min(_low, other.low()), glm::max(_high, other.high()));
}

void AABox::merge(const glm::dvec3 &point)
{
	 set(glm::min(point, _low), glm::max(point, _high));
}

bool AABox::operator==(const AABox& other) const
{
	return (_low == other.low()) && (_high == other.high());
}

bool AABox::operator!=(const AABox& other) const
{
	return !((_low == other.low()) && (_high == other.high()));
}

bool AABox::intersects(const AABox& other) const 
{
    // Must be overlap along all three axes.
    // Try to find a separating axis.

    for (int a = 0; a < 3; ++a) {

        //     |--------|
        // |------|

        if ((_low[a] > other.high()[a]) ||
            (_high[a] < other.low()[a])) {
            return false;
        }
    }

    return true;
}

double square(double val)
{
	return val*val;
}
