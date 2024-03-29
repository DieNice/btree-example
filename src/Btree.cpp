//
// Created by pda on 11/21/19.
//

#include "Btree.h"

using namespace std;

//********************************Node(Page)************************************
template<typename T>
Node<T>::Node(int pow) {
    t = pow;
    num = 0;
    leaf = true;
    links = new Node<T> *[2 * t];
    for (size_t i = 0; i < 2 * t; i++) {
        links[i] = nullptr;
    }
    keys = new T[2 * t - 1];
}

template<typename T>
Node<T>::~Node() {
    for (size_t i = 0; i < 2 * t; i++) {
        if (links[i] != nullptr)
            delete links[i];
    }
    delete[] links;
    delete[] keys;
}

template<typename T>
void Node<T>::splitChild(int i) {
    Node<T> *z = new Node<T>(t);
    Node<T> *y = links[i];
    z->leaf = y->leaf;
    for (int j = 0; j < t - 1; j++) {
        z->keys[j] = y->keys[j + t];
    }
    if (!y->leaf)
        for (int j = 0; j < t; j++) {
            z->links[j] = y->links[j + t];
            y->links[j + t] = nullptr;
        }
    y->num = t - 1;
    z->num = t - 1;
    for (int j = num; j > i; j--) {
        links[j + 1] = links[j];
    }
    links[i + 1] = z;
    for (int j = num - 1; j >= i; j--) {
        keys[j + 1] = keys[j];
    }
    keys[i] = y->keys[t - 1];
    num++;
}

template<typename T>
void Node<T>::insertNonFull(T k) {
    int i = num - 1;
    if (leaf) {
        while (i >= 0 && k < keys[i]) {
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i + 1] = k;
        num++;
    } else {
        while (i >= 0 && k < keys[i]) {
            i--;
        }
        i++;
        if (links[i]->num == 2 * t - 1) {
            splitChild(i);
            if (k > keys[i])
                i++;
        }
        links[i]->insertNonFull(k);
    }

}

template void Node<int>::insertNonFull(int k);

template void Node<char>::insertNonFull(char k);

template void Node<float>::insertNonFull(float k);

template<typename T>
void Node<T>::stealLeft(int i) {
    Node<T> *x = links[i];
    Node<T> *y = links[i - 1];
    T k = keys[i - 1];
    int xn = x->num;
    int yn = y->num;

    for (int j = xn; j > 0; j--) {
        x->keys[j] = x->keys[j - 1];
    }
    x->keys[0] = k;
    keys[i - 1] = y->keys[yn - 1];
    y->keys[yn - 1] = 0;

    for (int j = xn; j >= 0; j--) {
        x->links[j + 1] = x->links[j];
    }
    x->links[0] = y->links[yn];
    y->links[yn] = nullptr;

    x->num += 1;
    y->num -= 1;
}

template<typename T>
void Node<T>::stealRight(int i) {
    Node<T> *x = links[i];
    Node<T> *y = links[i + 1];
    T k = keys[i];
    int xn = x->num;
    int yn = y->num;

    x->keys[xn] = k;
    keys[i] = y->keys[0];
    for (int j = 0; j < yn - 1; j++) {
        y->keys[j] = y->keys[j + 1];
    }
    y->keys[yn - 1] = 0;

    x->links[xn + 1] = y->links[0];
    for (int j = 0; j < yn; j++) {
        y->links[j] = y->links[j + 1];
    }
    y->links[yn] = nullptr;

    x->num += 1;
    y->num -= 1;
}

template<typename T>
void Node<T>::mergeChild(int i) {
    Node<T> *y = links[i];
    Node<T> *z = links[i + 1];
    int yn = y->num;
    int zn = z->num;
    y->keys[yn] = keys[i];
    for (int j = 0; j < zn; j++) {
        y->keys[yn + j + 1] = z->keys[j];
    }
    if (!y->leaf)
        for (int j = 0; j <= zn; j++) {
            y->links[yn + 1 + j] = z->links[j];
            z->links[j] = nullptr;
        }
    y->num = yn + zn + 1;
    delete z;
    for (int j = i + 1; j < num; j++) {
        links[j] = links[j + 1];
    }
    links[num] = nullptr;
    for (int j = i; j < num - 1; j++) {
        keys[j] = keys[j + 1];
    }
    keys[num - 1] = 0;
    num--;
}

template<typename T>
T Node<T>::getMin(int i) {
    Node<T> *x = links[i];
    while (!x->leaf)
        x = x->links[0];
    return x->keys[0];
}

template<typename T>
T Node<T>::getMax(int i) {
    Node<T> *x = links[i];
    while (!x->leaf)
        x = x->links[x->num];
    return x->keys[x->num - 1];
}

template<typename T>
bool Node<T>::remove(T key) {
    int i = 0;
    for (i = 0; i < num; i++) {
        if (key <= keys[i]) {
            i++;
            break;
        }
        if (i == num - 1)
            i = num;
    }
    i--;
    if (i != num && key == keys[i]) {
        if (leaf) {
            for (int j = i; j < num - 1; j++) {
                keys[j] = keys[j + 1];
            }
            num--;
            keys[num] = 0;
            return true;
        } else {
            if (links[i]->num >= t) {
                T nkey = getMax(i);
                keys[i] = nkey;
                return links[i]->remove(nkey);
            } else if (links[i + 1]->num >= t) {
                T nkey = getMin(i + 1);
                keys[i] = nkey;
                return links[i + 1]->remove(nkey);
            } else {
                mergeChild(i);
                return links[i]->remove(key);
            }
        }

    } else {
        if (leaf) {
            return false;
        } else {
            if (links[i]->num == t - 1) {
                if (i > 0 && links[i - 1]->num >= t) {
                    stealLeft(i);
                } else if (i < num && links[i + 1]->num >= t) {
                    stealRight(i);
                } else {
                    if (i > 0) {
                        mergeChild(i - 1);
                        i--;
                    } else {
                        mergeChild(i);
                    }
                }
            }
            return links[i]->remove(key);
        }
    }
}

template bool Node<int>::remove(int key);

template bool Node<float>::remove(float key);

template bool Node<char>::remove(char key);


template<typename T>
void Node<T>::print(int d) {
    cout << "\nlvl=" << d << "=|";
    for (int i = 0; i < num; i++) {
        cout << keys[i] << "|";
    }

    if (!leaf) {
        for (int i = 0; i <= num; i++) {
            links[i]->print(d + 1);
        }
    }
}

template<typename T>
int Node<T>::search(T key) {
    int i = 0;
    for (i = 0; i < num; i++) {
        if (key <= keys[i]) {
            i++;
            break;
        }
        if (i == num - 1)
            i = num;
    }
    i--;
    if (i != num && key == keys[i]) {
        return i;
    } else if (leaf) { return -1; }
    else return links[i]->search(key);
}

template int Node<int>::search(int key);

template int Node<char>::search(char key);

template<>
int Node<float>::search(float key) {
    int i = 0;
    for (i = 0; i < num; i++) {
        if (key <= keys[i]) {
            i++;
            break;
        }
        if (i == num - 1)
            i = num;
    }
    i--;

    if (i != num && (abs(key - keys[i]) < 0.0001)) {
        return i;
    } else if (leaf) { return -1; }
    else return links[i]->search(key);
};


//**************************************BTREE**********************

template<typename T>
BTree<T>::BTree(int p) {
    power = p;
    root = nullptr;
}

template BTree<float>::BTree(int p);

template BTree<char>::BTree(int p);

template BTree<int>::BTree(int p);


template<typename T>
BTree<T>::~BTree() { delete root; }

template BTree<float>::~BTree();

template BTree<char>::~BTree();

template BTree<int>::~BTree();


template<typename T>
bool BTree<T>::search(T key) {
    if (root != nullptr) {
        int index = root->search(key);
        if (index >= 0) return true;
        return 0;
    }
    return 0;
}

template bool BTree<int>::search(int key);

template bool BTree<float>::search(float key);

template bool BTree<char>::search(char key);

template<typename T>
bool BTree<T>::add(T key) {
    if (search(key)) return true;
    if (root == nullptr) { root = new Node<T>(power); }
    Node<T> *r = root;
    if (root->num == 2 * power - 1) {
        Node<T> *s = new Node<T>(power);
        root = s;
        s->leaf = false;
        s->num = 0;
        s->links[0] = r;
        s->splitChild(0);
        s->insertNonFull(key);
    } else {
        r->insertNonFull(key);
    }
    return 0;
}

template bool BTree<float>::add(float key);

template bool BTree<char>::add(char key);

template bool BTree<int>::add(int key);


template<typename T>
void BTree<T>::print() {
    if (root != nullptr)
        root->print(0);
    else
        cout << "\nBTree is empty" << endl;

}

template void BTree<float>::print();

template void BTree<char>::print();

template void BTree<int>::print();

template<typename T>
bool BTree<T>::del(T key) {
    if (root == nullptr) return false;
    bool flag = root->remove(key);

    if (root->num == 0) {
        Node<T> *temp = root;
        root = temp->links[0];
        temp->links[0] = nullptr;
        delete temp;
    }

    return flag;
}

template bool BTree<int>::del(int key);

template bool BTree<float>::del(float key);

template bool BTree<char>::del(char key);