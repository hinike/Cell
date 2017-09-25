#include "scene_node.h"

#include "scene.h"

#include "../mesh/mesh.h"
#include "../shading/material.h"

#include <assert.h>

namespace Cell
{
    // --------------------------------------------------------------------------------------------
    SceneNode::SceneNode(unsigned int id) : m_ID(id)
    {
        
    }
    // --------------------------------------------------------------------------------------------
    SceneNode::~SceneNode()
    {
        // traverse the list of children and delete accordingly.
        for (unsigned int i = 0; i < m_Children.size(); ++i)
        {
            // it should not be possible that a scene node is childed by more than one
            // parent, thus we don't need to care about deleting dangling pointers.
            delete m_Children[i];
        }
    }
    // --------------------------------------------------------------------------------------------
    void SceneNode::SetPosition(math::vec3 position)
    {
        m_Position = position;
        m_Dirty = true;
    }
    // --------------------------------------------------------------------------------------------
    void SceneNode::SetRotation(math::vec4 rotation)
    {
        m_Rotation = rotation;
        m_Dirty = true;
    }
    // --------------------------------------------------------------------------------------------
    void SceneNode::SetScale(math::vec3 scale)
    {
        m_Scale = scale;
        m_Dirty = true;
    }
    // --------------------------------------------------------------------------------------------
    void SceneNode::SetScale(float scale)
    {
        m_Scale = math::vec3(scale);
        m_Dirty = true;
    }
    // --------------------------------------------------------------------------------------------
    math::vec3 SceneNode::GetPosition()
    {
        return m_Position;
    }
    // --------------------------------------------------------------------------------------------
    math::vec4 SceneNode::GetRotation()
    {
        return m_Rotation;
    }
    // --------------------------------------------------------------------------------------------
    math::vec3 SceneNode::GetScale()
    {
        return m_Scale;
    }
    // --------------------------------------------------------------------------------------------
    unsigned int SceneNode::GetID()
    {
        return m_ID;
    }
    // --------------------------------------------------------------------------------------------
    void SceneNode::AddChild(SceneNode *node)
    {
        // check if this child already has a parent. If so, first remove this scene node from its 
        // current parent. Scene nodes aren't allowed to exist under multiple parents.
        if (node->m_Parent)
        {
            node->m_Parent->RemoveChild(node->m_ID);
        }
        node->m_Parent = this;
        m_Children.push_back(node);
    }
    // --------------------------------------------------------------------------------------------
    void SceneNode::RemoveChild(unsigned int id)
    {
        auto it = std::find(m_Children.begin(), m_Children.end(), GetChild(id));
        if(it != m_Children.end())
            m_Children.erase(it);
    }
    // --------------------------------------------------------------------------------------------
    std::vector<SceneNode*> SceneNode::GetChildren()
    {
        return m_Children;
    }
    // --------------------------------------------------------------------------------------------
    unsigned int SceneNode::GetChildCount()
    {
        return m_Children.size();
    }
    // --------------------------------------------------------------------------------------------
    SceneNode *SceneNode::GetChild(unsigned int id)
    {
        for (unsigned int i = 0; i < m_Children.size(); ++i)
        {
            if(m_Children[i]->GetID() == id)
                return m_Children[i];
        }
        return nullptr;
    }
    // --------------------------------------------------------------------------------------------
    SceneNode* SceneNode::GetChildByIndex(unsigned int index)
    {
        assert(index < GetChildCount());
        return m_Children[index];
    }
    // --------------------------------------------------------------------------------------------
    SceneNode *SceneNode::GetParent()
    {
        return m_Parent;
    }
    // --------------------------------------------------------------------------------------------
    // NOTE(Joey): calculates the transform from its position, scale and 
    // rotation property. As the renderer generates the final transforms only 
    // once while filling the command buffer it is okay to calculate this on 
    // the spot (no dirty flags required).
    math::mat4 SceneNode::GetTransform()
    {
        if (m_Dirty)
        {
            UpdateTransform(false);
        }
        return m_Transform;
        //m_Transform = math::translate(m_Position);
        //m_Transform = math::scale(m_Transform, m_Scale);
        //m_Transform = math::rotate(m_Transform, m_Rotation.xyz, m_Rotation.w);

        // multiply parent transform with current transform.
        //if(m_Parent)
            //m_Transform = m_Parent->m_Transform * m_Transform; // note that we don't call GetTransform() as the parent's tranform should already be calculated; THIS IS RISKY! Think of better solution!

        //return m_Transform;
    }
    // --------------------------------------------------------------------------------------------
    math::mat4 SceneNode::GetPrevTransform()
    {
        return m_PrevTransform;
        //m_Transform = math::translate(m_Position);
        //m_Transform = math::scale(m_Transform, m_Scale);
        //m_Transform = math::rotate(m_Transform, m_Rotation.xyz, m_Rotation.w);

        // multiply parent transform with current transform.
        //if(m_Parent)
        //m_Transform = m_Parent->m_Transform * m_Transform; // note that we don't call GetTransform() as the parent's tranform should already be calculated; THIS IS RISKY! Think of better solution!

        //return m_Transform;
    }
    // --------------------------------------------------------------------------------------------
    void SceneNode::UpdateTransform(bool updatePrevTransform)
    {
        // we only do this if the node itself or its parent is flagged as dirty
        if (m_Dirty)
        {
            // first scale, then rotate, then translation
            m_Transform = math::translate(m_Position);
            m_Transform = math::rotate(m_Transform, m_Rotation.xyz, m_Rotation.w);
            m_Transform = math::scale(m_Transform, m_Scale);
            if (m_Parent)
            {
                m_Transform = m_Parent->m_Transform * m_Transform;
            }
            // if this node is flagged as dirty, also be sure to update its children as they have 
            // now become invalid as well
            for (int i = 0; i < m_Children.size(); ++i)
            {
                SceneNode* child = m_Children[i];
                child->m_Dirty = true;
                child->UpdateTransform();
            }
            m_Dirty = false;
        }
        // if specified, store current transform as prev transform (for calculating motion vectors)
        if (updatePrevTransform)
        {
            m_PrevTransform = m_Transform;
        }
    }
}