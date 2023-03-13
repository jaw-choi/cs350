#include "model.h"

void Model::Draw(Shader& shader, bool line)
{

    if (line == false)
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    else
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].DrawLine(shader);

}

BVHNode* BVHNode::buildBVH(const vector<BoundingBox>& object_bboxes, vector<int>& object_ids,int type_) {
    int n_objects = object_bboxes.size();
    vector<BVHNode*> leaves(n_objects);
    for (int i = 0; i < n_objects; i++) {
        leaves[i] = new BVHNode;
        leaves[i]->bbox = object_bboxes[i];
        leaves[i]->object_id = object_ids[i];
    }
    vector<BVHNode*> nodes(n_objects * 2 - 1);
    for (int i = 0; i < n_objects; i++)
        nodes[i] = leaves[i];

    int next_free_node = n_objects;
    while (next_free_node < n_objects * 2 - 1)
    {
        int left_child_index = -1;
        int right_child_index = -1;
        double min_volume = numeric_limits<double>::max();
        double min_distance = numeric_limits<double>::max();
        double min_delta_volume = numeric_limits<double>::max();
        int indexFirst = 0;
        int indexSecond = 0;
        for (int i = 0; i < next_free_node; i++)
        {
            if (nodes[i] == nullptr)
                continue;
            for (int j = i + 1; j < next_free_node; j++)
            {
                if (nodes[j] == nullptr)
                    continue;
                double volume = nodes[i]->bbox.getVolume() + nodes[j]->bbox.getVolume();
                double distance = glm::length(nodes[i]->bbox.center - nodes[j]->bbox.center);
                double delta_volume = nodes[i]->bbox.getVolume() - nodes[j]->bbox.getVolume();
                if(type_==0)
                {
                    if (volume < min_volume)
                    {
                        if (isNoLeaf(nodes, n_objects) > 1)
                        {
                            if (nodes[i]->object_id != -1 && nodes[j]->object_id != -1)
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_volume = volume;
                                min_distance = distance;
                            }
                        }
                        else
                        {
                            if (isNoLeaf(nodes, n_objects) == 1)
                            {
                                if (nodes[i]->object_id != -1 || nodes[j]->object_id != -1)
                                {
                                    left_child_index = i;
                                    right_child_index = j;
                                    min_volume = volume;
                                    min_distance = distance;

                                }

                            }
                            else
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_volume = volume;
                                min_distance = distance;

                            }
                        }
                    }
                }
                if (type_ == 1)
                {
                    if (distance < min_distance)
                    {
                        if (isNoLeaf(nodes, n_objects) > 1)
                        {
                            if (nodes[i]->object_id != -1 && nodes[j]->object_id != -1)
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_volume = volume;
                                min_distance = distance;
                            }
                        }
                        else
                        {
                            if (isNoLeaf(nodes, n_objects) == 1)
                            {
                                if (nodes[i]->object_id != -1 || nodes[j]->object_id != -1)
                                {
                                    left_child_index = i;
                                    right_child_index = j;
                                    min_volume = volume;
                                    min_distance = distance;

                                }

                            }
                            else
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_volume = volume;
                                min_distance = distance;

                            }
                        }
                    }
                }
                if (type_ == 2)
                {
                    if (delta_volume < min_delta_volume)
                    {
                        if (isNoLeaf(nodes, n_objects) > 1)
                        {
                            if (nodes[i]->object_id != -1 && nodes[j]->object_id != -1)
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_delta_volume = delta_volume;

                            }
                        }
                        else
                        {
                            if (isNoLeaf(nodes, n_objects) == 1)
                            {
                                if (nodes[i]->object_id != -1 || nodes[j]->object_id != -1)
                                {
                                    left_child_index = i;
                                    right_child_index = j;
                                    min_delta_volume = delta_volume;


                                }

                            }
                            else
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_delta_volume = delta_volume;


                            }
                        }
                    }
                }

            }
            
        }
        BVHNode* new_node = new BVHNode;
        new_node->left_child = nodes[left_child_index];
        new_node->right_child = nodes[right_child_index];
        new_node->bbox = BoundingBox(nodes[left_child_index]->bbox.mergeWith(nodes[right_child_index]->bbox));
        
        nodes[left_child_index] = nullptr;
        nodes[right_child_index] = nullptr;
        nodes[next_free_node++] = new_node;
    }
    return nodes[n_objects * 2 - 2];
}

BVHNode* BVHNode::buildBVHTopDown(const vector<BoundingBox>& object_bboxes, vector<int>& object_ids, int type_)
{
    int n_objects = object_bboxes.size();
    vector<BVHNode*> leaves(n_objects);
    for (int i = 0; i < n_objects; i++) {
        leaves[i] = new BVHNode;
        leaves[i]->bbox = object_bboxes[i];
        leaves[i]->object_id = object_ids[i];
    }
    vector<BVHNode*> nodes(n_objects * 2 - 1);
    for (int i = 0; i < n_objects; i++)
        nodes[i] = leaves[i];
    glm::vec3 MaxMedianCenter = glm::vec3(numeric_limits<double>::min(), numeric_limits<double>::min(), numeric_limits<double>::min());
    glm::vec3 MinMedianCenter = glm::vec3(numeric_limits<double>::max(), numeric_limits<double>::max(), numeric_limits<double>::max());
    glm::vec3 MedianCenter{};
    glm::vec3 MeanCenter{};
    for (int i = 0; i < n_objects; i++)
    {
        if (nodes[i]->bbox.center.x < MinMedianCenter.x)
            MinMedianCenter.x = nodes[i]->bbox.center.x;
        if (nodes[i]->bbox.center.y < MinMedianCenter.y)
            MinMedianCenter.y = nodes[i]->bbox.center.y;
        if (nodes[i]->bbox.center.z < MinMedianCenter.z)
            MinMedianCenter.z = nodes[i]->bbox.center.z;

        if (nodes[i]->bbox.center.x > MaxMedianCenter.x)
            MaxMedianCenter.x = nodes[i]->bbox.center.x;
        if (nodes[i]->bbox.center.y > MaxMedianCenter.y)
            MaxMedianCenter.y = nodes[i]->bbox.center.y;
        if (nodes[i]->bbox.center.z > MaxMedianCenter.z)
            MaxMedianCenter.z = nodes[i]->bbox.center.z;

        MeanCenter += nodes[i]->bbox.center;
    }
    MeanCenter /= n_objects;
    MedianCenter = (MinMedianCenter + MaxMedianCenter) / 2.f;

    int next_free_node = n_objects;
    while (next_free_node < n_objects * 2 - 1)
    {
        int left_child_index = -1;
        int right_child_index = -1;
        double min_volume = numeric_limits<double>::max();
        double min_distance = numeric_limits<double>::max();
        double min_delta_volume = numeric_limits<double>::max();
        int indexFirst = 0;
        int indexSecond = 0;
        for (int i = 0; i < next_free_node; i++)
        {
            if (nodes[i] == nullptr)
                continue;
            for (int j = i + 1; j < next_free_node; j++)
            {
                if (nodes[j] == nullptr)
                    continue;
                double volume = nodes[i]->bbox.getVolume() + nodes[j]->bbox.getVolume();
                double distance = glm::length(nodes[i]->bbox.center - nodes[j]->bbox.center);
                double delta_volume = nodes[i]->bbox.getVolume() - nodes[j]->bbox.getVolume();
                if (type_ == 0)
                {
                    if (volume < min_volume)
                    {
                        if (isNoLeaf(nodes, n_objects) > 1)
                        {
                            if (nodes[i]->object_id != -1 && nodes[j]->object_id != -1)
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_volume = volume;
                                min_distance = distance;
                            }
                        }
                        else
                        {
                            if (isNoLeaf(nodes, n_objects) == 1)
                            {
                                if (nodes[i]->object_id != -1 || nodes[j]->object_id != -1)
                                {
                                    left_child_index = i;
                                    right_child_index = j;
                                    min_volume = volume;
                                    min_distance = distance;

                                }

                            }
                            else
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_volume = volume;
                                min_distance = distance;

                            }
                        }
                    }
                }
                if (type_ == 1)
                {
                    if (distance < min_distance)
                    {
                        if (isNoLeaf(nodes, n_objects) > 1)
                        {
                            if (nodes[i]->object_id != -1 && nodes[j]->object_id != -1)
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_volume = volume;
                                min_distance = distance;
                            }
                        }
                        else
                        {
                            if (isNoLeaf(nodes, n_objects) == 1)
                            {
                                if (nodes[i]->object_id != -1 || nodes[j]->object_id != -1)
                                {
                                    left_child_index = i;
                                    right_child_index = j;
                                    min_volume = volume;
                                    min_distance = distance;

                                }

                            }
                            else
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_volume = volume;
                                min_distance = distance;

                            }
                        }
                    }
                }
                if (type_ == 2)
                {
                    if (delta_volume < min_delta_volume)
                    {
                        if (isNoLeaf(nodes, n_objects) > 1)
                        {
                            if (nodes[i]->object_id != -1 && nodes[j]->object_id != -1)
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_delta_volume = delta_volume;

                            }
                        }
                        else
                        {
                            if (isNoLeaf(nodes, n_objects) == 1)
                            {
                                if (nodes[i]->object_id != -1 || nodes[j]->object_id != -1)
                                {
                                    left_child_index = i;
                                    right_child_index = j;
                                    min_delta_volume = delta_volume;


                                }

                            }
                            else
                            {
                                left_child_index = i;
                                right_child_index = j;
                                min_delta_volume = delta_volume;


                            }
                        }
                    }
                }

            }

        }
        BVHNode* new_node = new BVHNode;
        new_node->left_child = nodes[left_child_index];
        new_node->right_child = nodes[right_child_index];
        new_node->bbox = BoundingBox(nodes[left_child_index]->bbox.mergeWith(nodes[right_child_index]->bbox));

        nodes[left_child_index] = nullptr;
        nodes[right_child_index] = nullptr;
        nodes[next_free_node++] = new_node;
    }
    return nodes[n_objects * 2 - 2];
}
