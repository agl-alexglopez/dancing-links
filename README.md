# Dancing Links

## Navigation

- Disaster Planning
	- Dancing Links Class **([`DisasterLinks.h`](/dancing-links/dlx/DisasterLinks.h))**
	- Dancing Links Implementation **([`DisasterLinks.cpp`](dancing-links/dlx/DisasterLinks.cpp))**
	- Traditional Set Implementation **([`DisasterPlanning.cpp`](/dancing-links/dlx/DisasterPlanning.cpp))** 
- Matchmaker (WIP)
	- Dancing Links Class (WIP)
	- Dancing Links Implementation (WIP)
	- Traditional Set Implementation **([`Matchmaker.cpp`](/dancing-links/dlx/Matchmaker.cpp))** 

## Build Note

In order to build this project, you will need a copy of the CS106 C++ library from the Winter of 2022. To the best of my knowledge, newer libraries from Stanford's class websites have some graphics conflicts that prevent this assignment from building. I have included a copy of this library as a `.zip` file in the `/packages/` folder. Unzip the folder in a separate location on your computer because it will need to built as its own Qt project. You can then follow the same instructions to build this library that Stanford normally provides on their course websites [HERE](https://web.stanford.edu/dept/cs_edu/resources/qt/). 

Instead of building their new libraries, however, you will just build this older version. It will install all the necessary Stanford specific libraries on your system for use with Qt.

## Overview

In October of 2022, Donald Knuth released *The Art of Computer Programming: Volume 4b: Combinatorial Algorithms, Part 2*. In this work he revised his previous implementation of his Algorithm X via Dancing Links. His revision included changing the doubly linked links that solved backtracking problems to an array of nodes. This is an interesting optimization. It provides good locality, ease of debugging, and memory safety if you use an array that does not need to be managed like a C++ vector. One of the points Knuth makes clear through his section on Dancing Links is that he wants to give people the tools they need to expand and try his backtracking strategies on many problems. If you want the original Algorithm, please read Knuth's work. Below, I will discuss how I applied or modified his strategies to the problems in this repository.


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

### Implementation

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

### Usage Instructions

I chose to implement the solution to this problem as a class that can be found in the **[`DisasterLinks.h`](/dancing-links/dlx/DisasterLinks.h)**, and **[`DisasterLinks.cpp`](/dancing-links/dlx/DisasterLinks.cpp)** files. There are tests at the bottom of the **[`DisasterLinks.cpp`](/dancing-links/dlx/DisasterLinks.cpp)** file that make the internals of the dancing links data structure more understandable.

I then included this implementation in the **[`DisasterGUI.cpp`](/dancing-links/dlx/Demos/DisasterGUI.cpp)** file as the solver for the maps included in this repository. If you want to see how this implementation solves the provided maps use the following steps.

1. Open the project in Qt Creator with the correct Stanford C++ library installed. (See the [Build Note](#build-note)).
2. Build and run the project.
3. Select the `Disaster Planning` option from the top menu.
4. Select any map from the drop down menu at the bottom of the window.
5. Press `Solve` to view the fewest number of cities that can cover the map.

## Dancing Partners

I will tackle the perfect matching problem with dancing links next. I have not gotten to this implementation yet.
