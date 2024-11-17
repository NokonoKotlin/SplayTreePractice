#include<iostream>
#include<cassert>


/*   
    このコメントは消さないでください。
    don't remove this comment!

    Copyright ©️ (c) NokonoKotlin (okoteiyu) 2024. 
    Released under the MIT license(https://opensource.org/licenses/mit-license.php) 
*/
template<class T , bool strong_debug = false >
class RangeSet{
    private:
    
    
    struct SplayNode{
        SplayNode *parent = nullptr;// parent node
        SplayNode *left = nullptr;// left child
        SplayNode *right = nullptr;// right child

        /*
            this node represents interval [ L , R )
            and has the sum of length of intarvals which nodes in its subtree represent.
        */

        T R;
        T L;
        T length_sum;

        // Size of Subtree under this node
        int SubTreeSize = 1;
                   
        private:
        bool copied_instance = false;
        public:

        SplayNode copy(){
            assert(copied_instance == false);
            SplayNode res = *this;
            res.left = nullptr;
            res.right = nullptr;
            res.parent = nullptr;
            res.copied_instance = true;
            return res;
        }

        SplayNode(){}

        SplayNode(T l , T r){
            assert(l<r);
            R = r;
            L = l;
            update();
        }

        // rotate ( this node - parent ) 
        void rotate(){
            if(this->parent->parent){
                if(this->parent == this->parent->parent->left)this->parent->parent->left = this;
                else this->parent->parent->right = this; 
            }
            if(this->parent->left == this){
                this->parent->left = this->right;
                if(this->right)this->right->parent = this->parent;
                this->right = this->parent;
                this->parent = this->right->parent;
                this->right->parent = this;
                this->right->update();
            }else{
                this->parent->right = this->left;
                if(this->left)this->left->parent = this->parent;
                this->left = this->parent;
                this->parent = this->left->parent;
                this->left->parent = this;
                this->left->update();
            }
            this->update();
            return;
        }

        // direction of this parent (left or right)
        int state(){
            if(this->parent == nullptr)return 0;
            if(this->parent->left == this)return 1;
            else if(this->parent->right == this)return 2;
            return 0;
        }

        // bottom-up splay 
        void splay(){
            while(bool(this->parent)){
                if(this->parent->state() == 0){
                    this->rotate();
                    break;
                }
                if( this->parent->state() == this->state() )this->parent->rotate();
                else this->rotate();
                this->rotate();
            }
            this->update();
            return;
        }

 
        // update data member
        void update(){
            assert(copied_instance == false);
            
            this->SubTreeSize = 1;
            this->length_sum = abs(this->R - this->L);
            if(bool(this->left)){
                this->SubTreeSize += this->left->SubTreeSize;
                this->length_sum += this->left->length_sum;
            }
            if(bool(this->right)){
                this->SubTreeSize += this->right->SubTreeSize;
                this->length_sum += this->right->length_sum;
            }
            return;
        }
 
    };
 


    /*
        order of node's R
        nodes (intervals) are sorted by the order of R
    */
    bool CompareNode(SplayNode *a , SplayNode *b ){return a->R <= b->R;}
 
    // get [index]th node pointer 
    SplayNode *get_sub(int index , SplayNode *root){
        if(root==nullptr)return root;
        SplayNode *now = root;
        while(true){
            int left_size = 0;
            if(now->left != nullptr)left_size = now->left->SubTreeSize;
            if(index < left_size)now = now->left;
            else if(index > left_size){
                now = now->right;
                index -= left_size+1;
            }else break;
        }
        now->splay();
        return now;
    }
 
    // merge 2 SplayTrees 
    SplayNode *merge(SplayNode *leftRoot , SplayNode *rightRoot){
        if(leftRoot!=nullptr)leftRoot->update();
        if(rightRoot!=nullptr)rightRoot->update();
        if(bool(leftRoot ) == false)return rightRoot;
        if(bool(rightRoot) == false )return leftRoot;
        rightRoot = get_sub(0,rightRoot);
        rightRoot->left = leftRoot;
        leftRoot->parent = rightRoot;
        rightRoot->update();
        return rightRoot;
    }
    
 
    // split SplayTree at [leftnum]
    std::pair<SplayNode*,SplayNode*> split(int leftnum, SplayNode *root){
        if(leftnum<=0)return std::make_pair(nullptr , root);
        if(leftnum >= root->SubTreeSize)return std::make_pair(root, nullptr);
        root = get_sub(leftnum , root);
        SplayNode *leftRoot = root->left;
        SplayNode *rightRoot = root;
        if(bool(rightRoot))rightRoot->left = nullptr;
        if(bool(leftRoot))leftRoot->parent = nullptr;
        leftRoot->update();
        rightRoot->update();
        return std::make_pair(leftRoot,rightRoot);
    }
 
    // remove [index]th node
    std::pair<SplayNode*,SplayNode*> Delete_sub(int index , SplayNode *root){
        if(bool(root) == false)return std::make_pair(root,root);
        root = get_sub(index,root);
        SplayNode *leftRoot = root->left;
        SplayNode *rightRoot = root->right;
        if(bool(leftRoot))leftRoot->parent = nullptr;
        if(bool(rightRoot))rightRoot->parent = nullptr;
        root->left = nullptr;
        root->right = nullptr;
        root->update();
        return std::make_pair(merge(leftRoot,rightRoot) , root );
    }
    
    /*
        between 2 SplayNodes [A] and [B] , we define following order.
        - [A] [<] [B] represent a order of these Keys.
        - [A] [==] [B] represent these Keys are same

        This function finds the border index [B] which satisfies following.
        1. if [lower] is true, for any [i] smaller than [B] , {[i]th node} [<] {[Node] argument}
        2. if [lower] is false, for any [i] smaller than [B] , {[i]th node} [<] {[Node] argument} or  {[i]th node} [==] {[Node] argument}
    */
    std::pair<SplayNode*,int> bound_sub(SplayNode* Node , SplayNode *root , bool lower){
        if(bool(root) == false)return std::make_pair(root,0);
        SplayNode *now = root;
        int res = 0;
        while(true){
            bool satisfy = CompareNode(now,Node); // upper_bound (now <= Node)
            if(lower)satisfy = !CompareNode(Node,now); // lower_bound (now < Node)
            if(satisfy){
                if(bool(now->right))now = now->right;
                else {
                    res++;
                    break;
                }
            }else{
                if(bool(now->left))now = now->left;
                else break;
            }
        }
        now->splay();
        if(bool(now->left))res += now->left->SubTreeSize;
        return std::make_pair(now ,res);
    }
    

    // insert [NODE]argument into SplayTree (in which nodes are sorted)
    SplayNode *insert_sub(SplayNode *NODE , SplayNode *root){
        NODE->update();
        if(bool(root) == false)return NODE;
        root = bound_sub(NODE,root,true).first;

        if(!CompareNode(NODE , root )){
            if(root->right != nullptr)root->right->parent = NODE;
            NODE->right = root->right;
            root->right = nullptr;
            NODE->left = root;
        }else{
            if(root->left != nullptr)root->left->parent = NODE;
            NODE->left = root->left;
            root->left = nullptr;
            NODE->right = root;
        }
        root->parent = NODE;
        root->update();
        NODE->update();
        return NODE;
    }
    

    // root node of this tree
    SplayNode *m_Root = nullptr;

    // bluff node object (used as temporary node)
    SplayNode *m_bluff_object = nullptr;
    SplayNode* BluffObject(T l , T r){
        if(m_bluff_object == nullptr)m_bluff_object = new SplayNode();
        m_bluff_object->L = l;
        m_bluff_object->R = r;
        return m_bluff_object;
    }

    void release(){
        while(size() > 0)Delete(0);
    }

    public:
    RangeSet(){}
    ~RangeSet(){release();}

    // don't copy this object
    RangeSet(const RangeSet &x) = delete ;
    RangeSet& operator = (const RangeSet&x) = delete ;
    RangeSet ( RangeSet&& x){assert(0);}
    RangeSet& operator = ( RangeSet&& x){assert(0);}
 
    // tree size
    int size(){
        if(m_Root==nullptr)return 0;
        return m_Root->SubTreeSize;
    }

    // get copy object of [i]th node (interval)
    // nodes (intervals) are sorted by the order of R
    SplayNode get(int i){
        assert(0 <= i && i < size());
        m_Root = get_sub(i,m_Root);
        return m_Root->copy();
    }


    // get copy object node which covers from [l]th node to [r-1]th node
    SplayNode GetRange(int l , int r){
        assert(0 <= l && l < r && r <= size());
        std::pair<SplayNode*,SplayNode*> tmp = split(r,m_Root);
        SplayNode* rightRoot = tmp.second;
        tmp = split(l,tmp.first);// 部分木を取り出す。
        SplayNode res = tmp.second->copy();
        m_Root = merge(merge(tmp.first,tmp.second),rightRoot);
        return res;
    }

    // delete [index]th interval
    void Delete(int index){
        if(index<0 || index >= size())assert(0);
        std::pair<SplayNode*,SplayNode*> tmp = Delete_sub(index,m_Root);
        m_Root = tmp.first;
        if(tmp.second != nullptr)delete tmp.second;
        return;
    }
    
    // counts nodes whose R < [x]
    int lower_bound(T x){
        if(size() == 0)return 0;
        std::pair<SplayNode*,int> tmp = bound_sub(BluffObject(x,x),m_Root,true);
        m_Root = tmp.first;
        return tmp.second;
    }
    
    // counts nodes whose R <=[x]
    int upper_bound(T x){
        if(size() == 0)return 0;
        std::pair<SplayNode*,int> tmp = bound_sub(BluffObject(x,x),m_Root,false);
        m_Root = tmp.first;
        return tmp.second;
    }

    // find the index [i] which [i]th interval covers [x].
    // if no answer is found, return -1
    int find_included(T x){
        int it_ = upper_bound(x);
        if(it_ < 0 || it_ >= size())return -1;
        if(get(it_).L <= x)return it_;
        return -1;
    }

    // whether two intervals [l1,r1) and [l2,r2) have intersection
    static constexpr bool intersect(T l1 , T r1 , T l2 , T r2){return (l1 <= l2 && l2 < r1) || (l2 <= l1 && l1 < r2);}
    // length of intersection of [l1,r1) and [l2,r2)
    static constexpr bool CommonArea(T l1 , T r1 , T l2 , T r2){
        if(!intersect(l1,r1,l2,r2))return 0;
        return min(r1,r2)-max(l1,l2);
    }
    
    // find one of existing intervals which [lef,rig) overlaps
    int find_cross(T lef , T rig){
        assert(lef < rig);
        int it_ = upper_bound(lef);
        if(it_ >= 0 && it_ < size()){
            SplayNode x = get(it_);
            if(intersect(lef,rig,x.L,x.R))return it_;
        }
        return -1;
    }
 
    // insert interval [lef,rig)
    void insert( T lef , T rig){
        assert(lef < rig);
        SplayNode x;
        while(true){
            int it_ = find_cross(lef,rig);
            if(it_ < 0 || it_ >= size())break;
            // if [strong debug], we cannot overlap existing interval
            if(strong_debug){
                std::cerr << "invalid : ( insert( " << lef << " , " << rig << " ) )" << std::endl;
                assert(false);
            }
            x = get(it_);
            lef = std::min(lef,x.L);
            rig = std::max(rig,x.R);
            Delete(it_);
        }
        int it_ = upper_bound(lef);
        for(int itr = it_ ; itr >= it_ - 1 ; itr-- ){
            if(itr >= 0 && itr < size()){
                x = get(itr);
                if(x.R == lef){
                    lef = std::min(lef,x.L);
                    Delete(itr);
                }else if(x.L == rig){
                    rig = std::max(rig,x.R);
                    Delete(itr);
                }
            }
        }
        m_Root = insert_sub(new SplayNode(lef , rig) ,m_Root);
        return;
    }
 
    
    // erase intersection of interval [lef,rig) and existing intervals
    void erase(T lef , T rig){
        assert(lef<rig);
        SplayNode x;
        while(true){
            int it_ = find_cross(lef,rig);
            if(it_ < 0 || it_ >= size())break;
            x = get(it_);
            Delete(it_);
            if(lef <= x.L && x.R <= rig)continue;
            else if(x.L <= lef && rig <= x.R){
                insert(x.L,lef);
                insert(rig,x.R);
            }else if(lef <= x.L && x.L < rig)insert(rig,x.R);
            else if(lef < x.R && x.R <= rig)insert(x.L,lef);
        }
        return;
    }
    
    /*   
        calculate the length of intersection of existing intervals and [lef,rig)
        we define the length of interval [l,r) as r-l .
    */
    T cover_length(T lef , T rig){
        assert(lef < rig);
        int it_lef = upper_bound(lef);
        if(it_lef == size())return 0;
        int it_rig = upper_bound(rig);
        T res = 0;
        SplayNode x = get(it_lef);
        if(x.L <= lef && rig <= x.R)return rig-lef;
        if(x.L<lef){
            it_lef++;
            res += x.R - lef;
        }
        if(it_lef < it_rig)res += GetRange(it_lef,it_rig).length_sum;
        if(it_rig < size()){
            x = get(it_rig);
            if(x.L < rig)res += rig-x.L;
        }
        return res;
    }

    // get [i]th interval.
    SplayNode operator [](int index){return get(index);}     
};


