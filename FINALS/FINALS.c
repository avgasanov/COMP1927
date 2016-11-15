'-----------------'
'R E C U R S I O N'
'-----------------'

====================================================================================
Recursion to find max val in a[lo..hi]
====================================================================================
int max(int a[], int lo, int hi)
{
	if (lo == hi) {					// #1 Max value found
		return a[lo];
	} else {
		if (a[lo] < a[hi])			// #2 LO < HI
			return max(a,lo+1,hi);		// increment lo++
		else 						// #3 LO > HI
			return max(a,lo,hi-1);		// decrement hi--
	}
}



'-----------'
'G R A P H S'
'-----------'

====================================================================================
NEIGHBOURS: Returns array of vertices that are neighbours of X
====================================================================================
// Neighbours
Vertex *neighbours(Graph g, Vertex x, int *nv)
{
	assert(validG(g) && validV(g,x) && nv != NULL);

	int i;
	int nneighbours = 0;
	for (i = 0; i < g->nV; i++) {		   // #1 Count num neighbours
		if (g->edges[x][i])
			nneighbours++;
	}

	Vertex *ns = malloc(nn * sizeof(Vertex)); // #2 Alloc neighbours array
	assert(ns != NULL);

	int k = 0;	
	for (i = 0; i < g->nV; i++) {		   // #3 Copies neighbour indexes
		if (g->edges[x][i])				   //    into neighbours array
			ns[k] = i;
			k++;
	}
	*nv = nneighbours;	// #neighbours
	return ns;			// return array of neighbours
}

====================================================================================
BFS: Adjacency Matrix Rep
====================================================================================
// BFS + global visited array
int *visited;	// array [0..V-1] of visiting order

void bfs(Graph g, Vertex v) {
	int i;
	int order = 1;
	visited = calloc(nV(g), sizeof(int));	// allocate visited array + set to 0
	Queue q = newQueue();					// create queue
	QueueJoin(q,v);							// add source to queue
	visited[v] = order++;					// mark source as visited
	while (!QueueIsEmpty(g)) {
		Vertex y;							// declare neighbour vertex
		Vertex curr = QueueLeave(q);			// pop vertex off queue
		if (visited[curr])						// check if vertex is visited
			continue;
		for (y = 0; y < nV(g); y++) {		// look at neighbours (ascending order)
			if (!hasEdge(g,curr,y))				// check if connected
				continue;
			if (!visited[y])					// check if neighbour is visited
				QueueJoin(q,y);					// add neighbour to queue
				visited[curr] = order++;			// mark curr vertice as visited
		}
	}
}

====================================================================================
DFS Recursive: Modified with Connected Components
====================================================================================
void dfs(Graph g) {
	int i;
	visited = calloc(nV(g), sizeof(int));
	order = 1;
	while (order < nV(g)) {				// When order = nV, we have visited all vertices
		Vertex v;
		for (v = 0; v < nV(g); v++)		// Check all vertices to see if they are visited
			if (visited[v] == 0) break;	// If not, do a recursive DFS
		dfsR(g,v);
	}
}
void dfsR(Graph g, Vertex x) {
	visited[v] = order++;				// Mark curr v as visited / the order visited			
	Vertex w;
	for (w = 0; w < nV(g); w++) {		// Iterate through possible neighbours W
		if (!hasEdge(g,v,w))				// For each W, check if there is an edge btwn
			continue;						// vertex V and W. Continue if no edge exists
		if (!visited[w])				// If W has not been visited, recursively visit W
			dfsR(g,w);						// Recursion will iterate through all possible	
	}										// neighbours Y. Once all neighours Y are visited
}											// then recursion will back-track one level

====================================================================================
DFS - CONNECTED COMPONENTS - Find # of connected components in a graph
====================================================================================
int nComponents(Graph g)
{
   int i, comp = 0; // Comp = COMPONENT #ID

   componentOf = malloc(g->nV*sizeof(int));			// #1 Allocate component array
   for (i = 0; i < g->nV; i++) componentOf[i] = -1; //    Initialise all = -1
   
   ncounted = 0;
   while (ncounted < g->nV) {				// #2 Search for non-visited vertices
      Vertex v;								//    If !visited, perform dfsR
      for (v = 0; v < g->nV; v++)			//    Increment COMP NO. / ID.
         if (componentOf[v] == -1) break;
      dfsComponents(g, v, comp);
      comp++;
   }										// Recursion in dfsR will make sure that
   // componentOf[] is now set 			    // all vertices in the same subgraph will
   return comp; 							// have the same compID, as they will be assigned
   											// the same ID. Only after searching through all
   											// vertices in the component
}
void dfsComponents(Graph g, Vertex v, int c)
{
   componentOf[v] = c;
   ncounted++;
   Vertex w;
   for (w = 0; w < g->nV; w++) {
      if (g->edges[v][w] && componentOf[w] == -1)
         dfsComponents(g, w, c);
   }
}

====================================================================================
DJIKSTRAS ALGORITHM
====================================================================================
void shortestPath(Graph g, Vertex start,
				  Vertex pred[], float dist[]) {

	PQueue pq = newPQ(dist,nV(g));			// #1 create PQ based on dist[] -> ptr access to dist[]

	for (Vertex v = 0; v < nV(g); v++) {	// #2 Init dist[] pred[] to base values for each vertex
		pred[v] = -1;						//    Add vertex to Queue
		dist[v] = MAX_WT;
		join(pq,v);
	}

	dist[start] = 0.0;						// #3 Begin at source vertex + reorder PQ
	reorder(pq,start);						//    (moves v to the front)

	while (!empty(pq)) {					// #4 PQ operations
		Vertex S = leave(pq);				//    Grab cheapest/front v in the queue
		/* FIND NEIGHBOURS */
		for (Vertex t = 0; t < nV(g); t++) {//    Iterate through other vertices
			float len = g->adj[s][t];		//    Find connected vertices to S (neighbours) + grab weight
			if (len == NO_EDGE) continue;	//    Length = non-existent, continue / check next T
			/* EDGE RELAXATION */
			if (dist[s]+len < dist[t]) {	//    If S cost so far + edge cost < known cost of path to T
				pred[t] = s;				//    Update new shortest path (previous of T = S)
				dist[t] = dist[s]+len;		//    Update new shortest dist (dist = dist[S] + len)
				reorder(pq,t);
			}
			/* NEXT ITERATION */
		}
	}
	dispose(PQ);
}


// 1. Minimum Spanning Trees -> A subset of edges that connects all vertices together using the
//                              minimum possible weight.

// 2. Cycle Checking -> Does a cycle exist in the graph?

// 3. Euler Path -> Path between 2 vertices V,W that goes through every edge at least ONCE
//					V,W = Odd degree (i.e. 1,3,5 edges connecting to vertice)
//					Remaining vertices = Even degree


'-----------------'
'B S T   T R E E S'
'-----------------'

====================================================================================
Higher Order Function - Passing & applying a function ptr to all nodes RECURSIVELY
====================================================================================
// Recursively apply function to nodes
void map(BSTree t, int(*f)(int))
{
	if (t != NULL) {			// While not empty
		t->value = f(t->value); // Apply function to val
		map(t->left,f);			// Recursive on LHS
		map(t->right,f);		// Recursive on RHS
	}
}

====================================================================================
BSTree Deletion: Wrapper + DeleteRoot
====================================================================================
// Wrapper function
BSTree BSTreeDelete(BSTree t, Key k)
{
	if (t == NULL) return;						// Empty tree
	if (k < t->value) BSTreeDelete(t->left);	// Key in LHS
	if (k > t->value) BSTreeDelete(t->right);	// Key in RHS
	if (k == t->value) t = deleteRoot(t); // Key found

	return t;
}
// Delete root
BSTree deleteRoot(t)
{
	Link newRoot;

	if (t->left == NULL && t->right == NULL) {		  // No subtrees
		free(t);
		return NULL;
	} else if (t->left == NULL && t->right != NULL) { // RHS subtree
		newRoot = t->right;							  // Ptr to LHS node
		free(t);
		return newRoot;
	} else if (t->left != NULL && t->right == NULL) { // LHS subtree
		newRoot = t->left;							  // Ptr to RHS node
		free(t);
		return newRoot;
	} else if (t->left != NULL && t->right != NULL) { // RHS + LHS exist
		Link curr = t->right; 						  // Find successor node
		while (curr != NULL) {
			curr = curr->left;
		}
		Key successor = curr->value;	// Grab successor key
		t = BSTreeDelete(t,successor);  // Remove successor node
		t->value = successor;			// Set root key = successor key
		return t;						// Return t
	}
}

====================================================================================
AVL TREES - Search Average/Worst = nLogn
====================================================================================
// Repairs imbalanced trees as soon as we notice imbalance
// Imbalanced = |depth(LHS) - depth(RHS) > 1|
Tree insertAVL(Tree t, Item it)
{
    if (t == NULL) return newNode(it);
    // Insert at leaves
    int diff = cmp(key(it), key(t->value));
    if (diff == 0) t->value = it;
    else if (diff < 0) t->left = insertAVL(t->left, it);
    else if (diff > 0) t->right = insertAVL(t->right, it);
    // Test imbalance
    int dL = TreeDepth(t->left);
    int dR = TreeDepth(t->right);
    if ((dL - dR) > 1) t = rotateR(t);
    else if ((dR - dL) > 1) t = rotateL(t);

    return t;
}

====================================================================================
GENERAL TREE REBALANCE - Recursive + Partition
====================================================================================
// Moving node with median key (i = n/2) to the root
Tree rebalance(Tree t)
{
	if (t == NULL) return NULL;
	// count nnodes in tree
	int n = count(t);
	if (n < 3) return t;
	// re-arrange tree so ith (n/2) element is root
	t = partition(t, n/2);
	// rebalance each subtree
	t->left = rebalance(t->left);
	t->right = rebalance(t->right);
	return t;
}
Tree partition(Tree t, int i)
{
   if (t == NULL) return NULL;
   assert(0 <= i && i < size(t));
   int n = size(t->left);
   // i'th in LHS + rotateR
   if (i < n) {
      t->left = partition(t->left, i);
      t = rotateR(t);
   }
   // i'th in RHS + rotateL
   if (i > n) {
      t->right = partition(t->right, i-n-1);
      t = rotateL(t);
   }
   t->nnodes = count(t); // fix count
   return t;
}

====================================================================================

// SPLAY TREES: Considers PARENT - CHILD - GRANDCHILD
//              Performs double rotations, pushing GRANDCHILD -> PARENT (ROOT)
//              E.g. P-C-G -> G-C-P

// 2-3-4 TREES: Three kinds of nodes
//              2-nodes = one value, two children (same as BST)
//              3-nodes = two values, three children
//              4-nodes = three values, four children
//              PROMOTE/SPLIT whenever node is full (contains 3 values)

====================================================================================
RED-BLACK TREES
====================================================================================
// Insertion Wrapper Function
void insertRedBlack(Tree t, Item it)
{
	t = insertRB(t->root, it, LEFT);	// start at root, insert item.
										// paramter #3 -> Go LHS or RHS
											// inRight = 0 -> Go LHS
											// inRight = 1 -> Go RHS
	t->colour = BLACK;
}

// Main insertion function (recursive)
Tree insertRB(Link t, Item it, Dirn dir)
{
   Key v = key(it);
   if (t == NULL) return newNode(it);
   if (red(L(t)) && red(R(t))) {
      t->colour = RED;
      t->left->colour = BLACK;
      t->right->colour = BLACK;
   }
   if (less(v, key(t->value))) {
      t->left = insertRB(t->left, it, LEFT);
      if (red(t) && red(L(t)) && dir==RIGHT)
         t = rotateR(t);
      if (red(L(t)) && red(L(L(t)))) {
         t = rotateR(t);
         t->colour = BLACK;
         t->right->colour = RED;
      }
   }
   else {
      t->right = insertRB(t->right, it, RIGHT);
      if (red(t) && red(R(t)) && dir==LEFT)
         t = rotateL(t);
      if (red(R(t)) && red(R(R(t)))) { 
         t = rotateL(t);
         t->colour = BLACK;
         t->left->colour = RED;
      }
    }
    return t;
}


'---------------------'
'H A S H   T A B L E S'
'---------------------'






