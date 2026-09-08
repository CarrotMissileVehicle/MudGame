//
// Created by z2996 on 2026/8/25.
//

#ifndef MUDGAME_BAG_H
#define MUDGAME_BAG_H
#include <string>
#include <vector>
#include "Object.h"

class Bag {
public:
    Bag() = default;
    ~Bag();

    // 深拷贝语义：复制堆叠对象（共享指针会引发双重释放）
    Bag(const Bag& other);
    Bag& operator=(const Bag& other);
    Bag(Bag&& other) noexcept;
    Bag& operator=(Bag&& other) noexcept;

    // 入包时同名物品自动堆叠合并（数量累加到已存在的堆叠上）
    void AddObject(Object* obj);
    // 不做堆叠合并直接入包：读档还原已聚合堆叠时使用
    void AddUnique(Object* obj);
    // 从同名堆叠扣减 count 个；数量归零则删除该堆叠；返回实际移除数量
    int RemoveObject(const std::string& name, int count = 1);
    [[nodiscard]] bool HasObject(const std::string& name) const;
    // 同名称物品的总数量（按堆叠累加）
    [[nodiscard]] int CountObject(const std::string& name) const;
    // 不同名称堆叠的数量
    [[nodiscard]] size_t GetSize() const;
    // 去重后的物品名称列表（每个堆叠一项，不含数量）
    [[nodiscard]] const std::vector<std::string> GetAllObjectName() const;
    // 带数量的展示列表：数量大于 1 显示 "名称 xN"，否则仅名称
    [[nodiscard]] const std::vector<std::string> GetStackedNames() const;
    [[nodiscard]] const std::vector<std::string> GetDescription() const;

    template<typename T>
    T *TryGetObjByType() {
        for (const auto obj: objects) {
            auto temp = dynamic_cast<T *>(obj);
            if (temp != nullptr)
                return temp;
        }
        return nullptr;
    }

    template<typename T>
    std::vector<T *> TryGetAllObjByType() {
        std::vector<T *> temp;
        for (const auto obj: objects) {
            auto tempObj = dynamic_cast<T *>(obj);
            if (tempObj != nullptr)
                temp.push_back(tempObj);
        }
        return temp;
    }

    const std::vector<Object*>& GetObjects() const { return objects; }

private:
    std::vector<Object *> objects;
};


#endif //MUDGAME_BAG_H
