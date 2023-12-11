#include "Transform2D.h"
#include <Matrix3.h>
#include <cmath>

Transform2D::Transform2D(Actor* owner)
{
    m_globalMatrix = new MathLibrary::Matrix3();
    m_localMatrix = new MathLibrary::Matrix3();
    m_translation = new MathLibrary::Matrix3();
    m_rotation = new MathLibrary::Matrix3();
    m_scale = new MathLibrary::Matrix3();
    m_children = nullptr;
    m_owner = owner;
}

Transform2D::~Transform2D()
{
    delete m_globalMatrix;
    delete m_localMatrix;
    delete m_rotation;
    delete m_translation;
    delete m_scale;
    delete[] m_children;
}

MathLibrary::Vector2 Transform2D::getForward()
{
    //Return the direction of the b x axis
    return MathLibrary::Vector2(m_globalMatrix->m00, m_globalMatrix->m10).getNormalized();
}

void Transform2D::setForward(MathLibrary::Vector2 value)
{
    //Get the direction the transform should rotate to face
    MathLibrary::Vector2 lookPosition = getWorldPosition() + value.getNormalized();

    //Rotate to face position
    lookAt(lookPosition);
}

MathLibrary::Vector2 Transform2D::getWorldPosition()
{
    //Return the translation column from the global matrix
    return MathLibrary::Vector2(m_globalMatrix->m02, m_globalMatrix->m12);
}

void Transform2D::setWorldPostion(MathLibrary::Vector2 value)
{
    //If the transform has a parent...
    if (m_parent)
    {
        //...convert the value into world coordinates and set the local position
        float xOffset = (value.x - m_parent->getWorldPosition().x) / MathLibrary::Vector2(getGlobalMatrix()->m00, getGlobalMatrix()->m10).getMagnitude();
        float yOffset = (value.y - m_parent->getWorldPosition().y) / MathLibrary::Vector2(getGlobalMatrix()->m10, getGlobalMatrix()->m11).getMagnitude();
        setLocalPosition({ xOffset, yOffset });
    }
    //Otherwise...
    else
        //...set the local position to the given value
        setLocalPosition(value);
}

MathLibrary::Vector2 Transform2D::getLocalPosition()
{
    //Return the translation column from the local matrix
    return MathLibrary::Vector2(m_localMatrix->m02, m_localMatrix->m12);
}

void Transform2D::setLocalPosition(MathLibrary::Vector2 value)
{
    //Set the translation matrix to a new matrix translated by the given amount
    *m_translation = MathLibrary::Matrix3::createTranslation(value);
    m_shouldUpdateTransforms = true;
}

void Transform2D::addChild(Transform2D* child)
{
    //Create a new array with a size one greater than our old array
    Transform2D** temp = new Transform2D * [m_childCount + 1];

    //Copy the values from the old array to the new array
    for (int i = 0; i < m_childCount; i++)
    {
        temp[i] = m_children[i];
    }
    delete[] m_children;

    //Set the last value in the new array to be the transform we want to add
    temp[m_childCount] = child;

    //Set the childs parent to be this transform
    child->m_parent = this;

    //Set old array to hold the values of the new array
    m_children = temp;

    //Increase the child count by one
    m_childCount + 1;
}

bool Transform2D::removeChild(int index)
{
    //Exit the function if the index was out of bounds
    if (index < 0 || index >= m_childCount)
        return false;

    //Create variable to store if the child was removed
    bool childRemoved = false;

    //Create a new temporary array with a size one less than our old array
    Transform2D** temp = new Transform2D * [m_childCount - 1];

    //Create variable to access temporary array index
    int j = 0;

    //Copy values from the old array to the new array except the child to delete
    for (int i = 0; i < m_childCount; i++)
    {
        //If the child to delete was skipped, set the child removed variable to true.
        if (i == index)
        {
            delete[] m_children;

            childRemoved = true;
            continue;
        }

        temp[j] = m_children[i];
        j++;
    }

    //Set the old array to the new array and decrease the child count if the child was removed
    if (childRemoved)
    {
        m_children = temp;
        m_childCount--;
    }

    //Set the child parent to null
    m_children[index]->m_parent == nullptr;

    //Return whether or not the removal was successful
    return childRemoved;
}

bool Transform2D::removeChild(Transform2D* child)
{
    //Exit the function if the child is null
    if (!child)
        return false;

    //Create variable to store if the child was removed
    bool childRemoved = false;

    //Create a new temporary array with a size one less than our old array
    Transform2D** temp = new Transform2D * [m_childCount - 1];

    //Create variable to access temporary array index
    int j = 0;

    //Copy values from the old array to the new array except the child to delete
    for (int i = 0; i < m_childCount; i++)
    {
        //If the child to delete was skipped, set the child removed variable to true.
        if (m_children[i] == child)
        {
            childRemoved = true;
            continue;
        }

        temp[j] = m_children[i];
        j++;
    }

    //Set the old array to the new array and decrease the child count if the child was removed
    if (childRemoved)
    {
        m_children = temp;
        m_childCount--;
    }

    //Set the child parent to null
    child->m_parent == nullptr;

    //Return whether or not the removal was successful
    return childRemoved;
}

void Transform2D::setScale(MathLibrary::Vector2 scale)
{
    //Set the scale matrix to a new matrix scaled by the given amount
    *m_scale = MathLibrary::Matrix3::createScale(scale);
    m_shouldUpdateTransforms = true;
}

void Transform2D::scale(MathLibrary::Vector2 scale)
{
    //Combine the scale matrix with a new matrix scaled by the given amount
    *m_scale = *m_scale * MathLibrary::Matrix3::createScale(scale);
    m_shouldUpdateTransforms = true;
}

MathLibrary::Vector2 Transform2D::getScale()
{
    //Get the magnitude of the x and y axis
    float magnitudeX = MathLibrary::Vector2({ m_scale->m00, m_scale->m10 }).getMagnitude();
    float magnitudeY = MathLibrary::Vector2({ m_scale->m01, m_scale->m11 }).getMagnitude();

    return MathLibrary::Vector2({ magnitudeX, magnitudeY });
}

void Transform2D::setRotation(float radians)
{
    ////Set the rotation matrix to a new matrix rotated by the given amount
    *m_rotation = MathLibrary::Matrix3::createRotation(radians);
    m_shouldUpdateTransforms = true;
}

void Transform2D::rotate(float radians)
{
    //Combine the rotation matrix with a new matrix rotated by the given amount
    *m_rotation = *m_rotation * MathLibrary::Matrix3::createRotation(radians);
    m_shouldUpdateTransforms = true;
}

void Transform2D::lookAt(MathLibrary::Vector2 position)
{
    //Find the direction that the actor should look in
    MathLibrary::Vector2 direction = (position - getWorldPosition()).getNormalized();

    //Use the dotproduct to find the angle the actor needs to rotate
    float dotProd = MathLibrary::Vector2::dotProduct(direction, getForward());
    if (abs(dotProd) > 1)
        dotProd = 1;
    float angle = (float)acos(dotProd);

    //Find a perpindicular vector to the direction
    MathLibrary::Vector2 perp = MathLibrary::Vector2(direction.y, -direction.x);

    //Find the dot product of the perpindicular vector and the current forward
    float perpDot = MathLibrary::Vector2::dotProduct(perp, getForward());

    //If the result isn't 0, use it to change the sign of the angle to be either positive or negative
    if (perpDot != 0)
        angle *= -perpDot / abs(perpDot);

    rotate(angle);
}

MathLibrary::Matrix3* Transform2D::getGlobalMatrix()
{
    return m_globalMatrix;
}

MathLibrary::Matrix3* Transform2D::getLocalMatrix()
{
    return m_localMatrix;
}

void Transform2D::updateTransforms()
{
    //Combine the translation, rotation, and scale matrices to form the local matrix
    *m_localMatrix = *m_translation * *m_rotation * *m_scale;

    //If the transform has a parent...
    if (m_parent)
        //...set the global matrix to be the parent global combined with the local
        *m_globalMatrix = *(m_parent->m_globalMatrix) * (*m_localMatrix);
    //Otherwise...
    else
        //...set the global to be the local
        *m_globalMatrix = *m_localMatrix;
}
