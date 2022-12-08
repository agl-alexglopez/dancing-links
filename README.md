# Dancing Links

## Navigation

- Disaster Planning
	- Dancing Links Class **([`DisasterLinks.h`](/dancing-links/dlx/DisasterLinks.h))**
	- Dancing Links Implementation **([`DisasterLinks.cpp`](dancing-links/dlx/DisasterLinks.cpp))**
	- Traditional Set Implementation **([`DisasterPlanning.cpp`](/dancing-links/dlx/DisasterPlanning.cpp))** 
- Matchmaker
	- Dancing Links Class **([`PartnerLinks.h`](/dancing-links/dlx/PartnerLinks.h))**
	- Dancing Links Implementation **([`PartnerLinks.cpp`](/dancing-links/dlx/PartnerLinks.cpp))**
	- Traditional Set Implementation **([`Matchmaker.cpp`](/dancing-links/dlx/Matchmaker.cpp))** 

## Build Note

In order to build this project, you will need a copy of the CS106 C++ library from the Winter of 2022. To the best of my knowledge, newer libraries from Stanford's class websites have some graphics conflicts that prevent this assignment from building. I have included a copy of this library as a `.zip` file in the `/packages/` folder. Unzip the folder in a separate location on your computer because it will need to built as its own Qt project. You can then follow the same instructions to build this library that Stanford normally provides on their course websites [HERE](https://web.stanford.edu/dept/cs_edu/resources/qt/). 

Instead of building their new libraries, however, you will just build this older version. It will install all the necessary Stanford specific libraries on your system for use with Qt.

## Overview

In October of 2022, Donald Knuth released *The Art of Computer Programming: Volume 4b: Combinatorial Algorithms, Part 2*. In this work he revised his previous implementation of his Algorithm X via Dancing Links. His revision included changing the doubly linked links that solved backtracking problems to an array of nodes. This is an interesting optimization. It provides good locality, ease of debugging, and memory safety if you use an array that does not need to be managed like a C++ vector. One of the points Knuth makes clear through his section on Dancing Links is that he wants to give people the tools they need to expand and try his backtracking strategies on many problems. If you want the original Algorithm, please read Knuth's work. Below, I will discuss how I applied or modified his strategies to the problems in this repository. I solve three problems: supplying a transportations grid, perfect matching for partners, and maximum weight matching for partners.


## Dancing with Danger

Let's look at transportation grids. Take the following grid as an example. Letters represent cities and they are connected to other cities in an undirected graph. There are no costs for the connections.

![grid-1](/images/grid-1.png)

We are given a certain number of supplies with which we can cover this grid. Let's start with 2 supplies. Can we cover this grid? A city is covered under the following conditions.

- A city has supplies.
- A city is adjacent to a city that has supplies.
- A city may only be supplied once.
- If a city is adjacent to a city with supplies, thus making it covered, it may still receive supplies to help other cities.

The answer in this grid is fairly obvious. Supply cities D and B.

![grid-1-solved](/images/grid-1-solved.png)

However, how can we use Knuth's methods to represent this grid and solve the problem?

### The Grid

We can start by placing these cities in a matrix configuration, an `NxN` grid. Let's make the columns the name of every city that exists in the transportation grid we are trying to cover. Then, we will make the rows those same cities. However, as rows, they will represent all of the other cities to which they are connected. Here is the scheme.

![matrix-grid](/images/matrix-grid.png)

Here are the key details from the above matrix representation of the grid.

- We distinguish the cities as items and cities as supplying options with the open bracket `{`.
- A city is in its own set of connections. This is important in case a city is an island with no connections that must be given supplies.
- Intuitively, items that are hard to reach will make few appearances across all supply locations. This can be seen by looking down a single column and counting the appearance of `1`'s

So, we now need to attempt to cover a city. Because this is a recursive backtracking problem, our selection should make the problem a smaller version of the same problem. To select a city to attempt to cover we will apply our first optimization. Here is the heuristic.

- Scan all items that remain in the network as items to be covered.
- Select the the city that appears the fewest number of times across all sets. We will start by trying to cover isolated cities.
- Try to supply this city by either giving it supplies or giving its neighbor supplies.
- Try every supply location. If none work with the given number of supplies, coverage of the network is impossible.

We can now take a look at how our selection shrinks the problem. If we cover city A by supplying city A, here is what happens to our grid.

![matrix-grid-shrink](/images/matrix-grid-shrink.png)

Here are the key details from the above image:

- City A and D have been covered, meaning they are safe from danger.
- However, city A is the only city that can no longer receive supplies. City D is still available to supply as an option, and it will be able to help the cities that remain uncovered.

We will soon discover that with only two supplies A was a bad choice and we will restore the grid to its original state. Let's fast forward to the correct choices and observe how they solve the problem in this format.

![matrix-grid-solve](/images/matrix-grid-solve.png)

Here are the key details from the above image:

- Good supply choices can make the problem significantly smaller.
- Even though B was covered by supplying D, it was still available to supply E and F, thus solving the problem with the restriction of two supplies.
- We know the problem is solved when no cities remain as columns.
- A side effect of this selection strategy is that we will have empty options during recursion. This is not a problem however because the remaining cities are never aware of options that cannot reach them and this does not slow down the speed of the program.

While the grid setup and selection process I have outlined will solve the problem, we can improve upon it slightly.

### Optimizations

The previous selection process will always start by covering a city by giving it supplies. We already know that we will start by trying to cover the most isolated city. Is it a good idea to try to give that city supplies first? I don't think so. Let's adjust the selection heuristic.

- Select the most isolated city so far to attempt to cover.
- Try to cover this city by giving the most well connected neighbor supplies first.
- If that does not work try the next most connected neighbor.
- If all else fails, give the isolated city supplies.
- If that fails the grid cannot be covered.

There is no need to adjust the dancing links algorithm to achieve this. Instead we will alter how we build the grid. Organize the rows of the grid by the cities with the most connections to least. Then, our selection process will naturally follow the heuristic. 

![matrix-grid-optimized](/images/matrix-grid-optimized.png)

As an exercise, trace through the same selection process to shrink this world and see if you notice any differences in how quickly it solves the problem.

As a final note for this section, I am seeking further optimizations. Currently, this implementation can solve all maps except the U.S. grid instantly. My algorithm can confirm that the optimal number of cities to supply lies between 20 and 30. We cannot cover the U.S. with less than 20 supplies and we can cover the U.S. with 30 or more supplies. However, finding the optimal number within this range has proven too much for my implementation. Finding the solution takes too much time.

### Disaster Links Implementation

I will briefly go over the new data structure that Knuth recommends for these types of problems because it is quite interesting. For a deep dive, see the code and test cases in `DisasterLinks.cpp`.

It might appear from the previous section that we use a traditional `NxN` grid to solve these problems. The issue with that is that for any non trivial network, the grid would be quite sparse and waste space.

Instead we can fit all of the logic for this algorithm into one array. Because this is implemented in `C++`, I use a `vector` to organize this data. Here is how I organized what I needed to solve this problem.

- Create a lookup table of city names so that we can collect the names of cities that we supply as an output parameter during recursion.
- Create the actual grid array. This array will contain item nodes.
- Item nodes have the following fields: `topOrLen`, `up`, `down`, `left`, `right`.
- If an item node is the header for a column, the first field is the length of the column so we can select an isolated city to cover.
- If an item node is in a column, its first field points back up to the column header.
- Every row of the grid will be signified by a spacer node with a negative `topOrLen` field. This field tells us which city we are supplying in the given option.
- Every column is a circular doubly linked list and every row is a circular doubly linked list.
- Instead of pointers to other nodes, these directional fields store indices to other locations in the array.
- Recursion occurs with the data structure remaining in place. No copies are necessary.

Here is the struct for the type we place in the lookup table.

```c++
typedef struct cityHeader {
    std::string name;
    int left;
    int right;
}cityHeader;
```

This is the struct I place in the dancing links grid. It is similar to a normal node with pointers, we just point to indices in the array.

```c++
typedef struct cityItem {
    int topOrLen;
    int up;
    int down;
    int left;
    int right;
}cityItem;
```

I then pack these data structures into a struct. The `numItemsAndOptions` is just the number of cities we must cover. There is an equivalent number of cities as items to cover and cities as supply options.

```c++
typedef struct Network {
    Vector<cityHeader> lookupTable;
    Vector<cityItem> grid;
    int numItemsAndOptions;
}Network;
```

Here is a literal representation of the array and how it appears in memory for the example I previously illustrated.

The lookup table that controls recursion and helps us collect supply location names is a separate array.

![lookup-table-illustrated](/images/lookup-table-illustrated.png)

The core logic of the algorithm takes place in this array where items exist in all of the sets they are connected to.

![matrix-array-illustrated](/images/matrix-array-illustrated.png)

There are some leftovers from Knuth's logic that I am not sure what to do with in this problem and don't play a role to solve the supply problem. Compare this image to the illustration in Knuth's work and the table he produces of his array to see more differences.

Unfortunately, I cannot take advantage of the implicit left right nature of array elements next to one another in this implementation. I need explicit `left` `right` fields because I remove cities from all other options that they appear in without eliminating the other options for future supply possibilities. This means I must splice them out of a doubly linked list.

### Disaster Links Usage Instructions

I chose to implement the solution to this problem as a class that can be found in the **[`DisasterLinks.h`](/dancing-links/dlx/DisasterLinks.h)**, and **[`DisasterLinks.cpp`](/dancing-links/dlx/DisasterLinks.cpp)** files. There are tests at the bottom of the **[`DisasterLinks.cpp`](/dancing-links/dlx/DisasterLinks.cpp)** file that make the internals of the dancing links data structure more understandable.

I then included this implementation in the **[`DisasterGUI.cpp`](/dancing-links/dlx/Demos/DisasterGUI.cpp)** file as the solver for the maps included in this repository. If you want to see how this implementation solves the provided maps use the following steps.

1. Open the project in Qt Creator with the correct Stanford C++ library installed. (See the [Build Note](#build-note)).
2. Build and run the project.
3. Select the `Disaster Planning` option from the top menu.
4. Select any map from the drop down menu at the bottom of the window.
5. Press `Solve` to view the fewest number of cities that can cover the map.

## Dancing Partners

This section covers the use of Knuth's method to solve graph matching problems. Specifically, I cover two problems: Perfect Matching and Max Weight Matching. When I originally heard Knuth speak about his Dancing Links technique, he mentioned Perfect Matching as an ideal application of his methods. Subsequently, when I read his work I saw that he was interested in generating every possible way to cover a graph. Instead, we will tweak his methods slightly again to achieve slightly easier tasks. These implementations are more in line with what Knuth had in mind, when compared to the disaster planning algorithm discussed in the last section.

### Perfect Matching

Let's look at another graph. This time, the graph will represent people. The connections, or edges, between people are bidirectional and carry no weight.

![pairs-1](/images/pairs-1.png)

How can we match everyone in this graph so that nobody is left out? There are a few ways, here is one.

![pairs-solve-1](/images/pairs-solve-1.png)

There would also be bad choices for partners. For example, if we paired A-D and B-C, E and F would have no partners when we could have matched them. Avoiding such bad choices is the challenge of this problem. We can now represent this graph as a matrix with Knuth's method in mind. However, we will alter our approach as compared to the last section.

![pairs-matrix-1](/images/pairs-matrix-1.png)

Here are the key details from the above matrix.

- Every person appears as an option along the top headers.
- We only allow for two people for every option, the rows of the matrix.
- Every option is symmetric and unique. While maps may sometimes confirm a pairing across multiple keys, we only need to place them in this matrix once.
- Option row titles have no meaning. They are arbitrary numbers and could be used to give a title to every pairing if needed. My solution does not need to do this.

We now must select a person we will attempt to match. The selection is simple.

- Scan all options to confirm that everyone is accessible through a partnership.
	- If someone has 0 appearances across all options, that means a previous pairing was a bad choice because it left someone stranded. We will stop recursing and backtrack to try a better choice.
- Select the first available person and try every way to match them.
- One of the ways to match the person will work.

We know that a person will find a successful match because we find any singletons or odd sized graphs while building the matrix. Therefore, we do not even need to try to match anyone in those cases because we know it will be impossible and do not enter recursion.

Compared to the previous disaster planning problem, covering a person creates a different effect on the matrix. Here is how selection shrinks the problem.

![pairs-cover-A](/images/pairs-cover-A.png)

Here are the key details from the above image:

- We select to pair A with D, marked by the green asterisk (`*`).
- A and D disappear as people that need to be covered by hiding their columns.
- All other options affected by this pairing are colored green.
- A-F and D-E are no longer valid pairings because A and D are paired off.

To complete this pairing we have four more people to pair. Let's look at how a bad choice changes the matrix.

![pairs-bad-cover](/images/pairs-bad-cover.png)

Here are the key details from the above image:

- Selecting to pair B-C leaves E and F isolated.
- We know E and F cannot be paired because the headers will point to themselves and have no options to access.
- To solve a Perfect Matching problem, all options and items must disappear from the world with the last choice.
- The problem is solved when we have an empty world or matrix.

I am not currently aware of any optimizations for this problem. It is very fast at finding solutions, especially because it only must find one. In, contrast max weight matching is much more difficult.

### Max Weight Matching

Here is another graph. Again, the nodes are people and the edges are the connections showing their willingness to pair with that person. However the edges have a weight.

![weight-1](/images/weight-1.png)

It is clear to see how to match everyone so nobody is left out. However, complete that matching and total the numbers shared between the pairs. You will see that we leave out a valuable edge between B and C. Here is the correct choice to maximize edge weight.

![weight-1-solved](/images/weight-1-solved.png)

Here are the key details from the above graph:

- B-C and A-F produce a much higher weight than a Perfect Matching.
- Max Weight matching allows for people to be left out, meaning it will work on any sized graph.

We will represent this graph in a matrix almost identically to our Perfect Matching problem.

![weight-matrix-1](/images/weight-matrix-1.png)

Here are the key details from the above image:

- People remain as items across the columns.
- We only allow pairs as options.
- The options are given a title that includes the weight of the pair.

Because this problem asks a slightly different question of us, we need to develop one new method for covering people and eliminating options.

To find the Maximum Weight Matching, we must explore every possible pairing and report back the best one we found. My best attempt at this is using what is called an include/exclude pattern. Here is how it goes in abstract.

- Select the next available person.
- Exclude them from other options as a possible partner and recurse.
- Include them and try every possible way of pairing them off with their possible partners.
- From these two choices keep the result of the best choice that led to the highest weight matching.

Step 2 of the above algorithm is the only one that requires a new method of covering a person in the matrix. Here is what it looks like.

![matrix-cover-person](/images/matrix-cover-person.png)

Here are the key details of the above image:

- The person we are covering is marked by the asterisk (A).
- When we cover an individual person, the only options that disappear are the pairings that that person can affect by being absent.
- We do NOT remove all options that include this person's partner. For example, we do not remove all other options that have B or C as a possible partner.
- F is unreachable and that is completely acceptable. In fact, we will continue to attempt combinations of partners until the world is empty or nobody can be paired.

We will then continue the process of excluding and including each person at every level of recursion to produce every possible pairing and record the best weight as we go.

Trying to explain or trace through the generation of pairings beyond this level of detail can become confusing, so I will leave it at that. This implementation definitely requires a recursive leap of faith.

This is the slower of the two implementations and I am not aware of any optimizations at this time. However, I am sure they exist and will try to incorporate them as I learn.

### Partner Links Implementation

Luckily, both of the Matching problems can use the exact same types and data structures to solve their problems. We only alter the algorithm's logic when necessary.

We again pack all of the nodes into one array. We use a lookup table array and the actual array that contains the dancing links logic.

Here is the type that goes in the lookup table and controls our recursion, telling us when all people have been partnered or who to pick next.

```c++
typedef struct personName {
	std::string name;
	int left;
	int right;
}personName;
```

Here is the type we place in the array that we walk through during recursion. The array remains in place, we only ever alter the fields of the nodes. As a header this types `topOrLen` will store the number of options we have to access a pairing for a person. As an item in a column, the `topOrLen` will point back up to the header. It can also give us the name of that item in the lookup table. The left and right fields of the node are implicit because pairs are next to one another in an array.

```c++
typedef struct personLink {
	int topOrLen;
	int up;
	int down;
}personLink;
```

I place the two arrays and some extra logic information in a struct called a Network.

```c++
typedef struct Network {
	Vector<personName> lookupTable;
	Vector<personLink> links;
	int numPeople;     // Total people in the network.
	int numPairings;   // The number of pairings or rows in the matrix.
	bool hasSingleton; // No perfect matching if someone is alone.
	bool isWeighted;   // Must provide weights for max weight matching.
}Network;
```

The lookup table is identical to the disaster planning problem.

![lookup-table-illustrated](/images/lookup-table-illustrated.png)

Our Dancing Links array will look slightly different in these implementations. Here is the array used for Perfect Matching.

![perfect-matrix-illustrated](/images/perfect-matrix-illustrated.png)

We always know that any other nodes for a pairing are directly next to one another in the array. The array has one modification for the maximum weight matching.

![weight-matrix-array-illustrated](/images/weight-matrix-array-illustrated.png)

Notice that we include the weight for a pairing in as a negative number. This tells us the weight we need and also that we have arrived at a spacer node, not a partner node.

### Partner Links Usage Instructions

I chose to implement the solution to this problem as a class that can be found in the **[`PartnerLinks.h`](/dancing-links/dlx/PartnerLinks.h)**, and **[`PartnerLinks.cpp`](/dancing-links/dlx/PartnerLinks.cpp)** files. There are tests at the bottom of the **[`PartnerLinks.cpp`](/dancing-links/dlx/DisasterLinks.cpp)** file that make the internals of the dancing links data structure more understandable.

I then included this implementation in the **[`MatchmakerGUI.cpp`](/dancing-links/dlx/Demos/MatchmakerGUI.cpp)** file as the solver for grid illustration tool. If you want to see how this implementation finds matches complete the following steps.

1. Open the project in Qt Creator with the correct Stanford C++ library installed. (See the [Build Note](#build-note)).
2. Build and run the project.
3. Select the `Matchmaker` option from the top menu.
4. Draw a graph or select a premade example from the load option.
5. Select `Find Perfect Matching` or `Find Max-Weight Matching` to solve the appropriate problem.

## Runtime Analysis

I am curious how these implementations stack up against the original solutions I wrote to these problems. I will put together a more formal runtime analysis when able.
