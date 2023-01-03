# Dancing Links

## Navigation

- Disaster Planning
	- Dancing Links Class **([`DisasterLinks.h`](/dancing-links/dlx/DisasterLinks.h))**
	- Dancing Links Implementation **([`DisasterLinks.cpp`](dancing-links/dlx/DisasterLinks.cpp))**
	- Traditional Set Implementation **([`DisasterPlanning.cpp`](/dancing-links/dlx/DisasterPlanning.cpp))** 
	- Dancing Links with Supply Tags Class **([`DisasterTags.h`](/dancing-links/dlx/DisasterTags.h))**
	- Dancing Links with Supply Tags Implementation **([`DisasterTags.cpp`](/dancing-links/dlx/DisasterTags.cpp))**
- Matchmaker
	- Dancing Links Class **([`PartnerLinks.h`](/dancing-links/dlx/PartnerLinks.h))**
	- Dancing Links Implementation **([`PartnerLinks.cpp`](/dancing-links/dlx/PartnerLinks.cpp))**
	- Traditional Set Implementation **([`Matchmaker.cpp`](/dancing-links/dlx/Matchmaker.cpp))**
- Pokémon Planning
	- Dancing Links Class **([`PokemonLinks.h`](/dancing-links/dlx/PokemonLinks.h))**
	- Dancing Links Implementation **([`PokemonLinks.cpp`](/dancing-links/dlx/PokemonLinks.cpp))**
- [Citations](#citations)

## Build Note

If you are reading this now you are on `gitfront.io`. If you want to use the programs for this project, you can clone the project with the `clone` button at the top right of the screen when in the root of the repository. In order to build this project, you will need a copy of the CS106 C++ library from the Winter of 2022. To the best of my knowledge, newer libraries from Stanford's class websites have some graphics conflicts that prevent this assignment from building. I have included a copy of this library as a `.zip` file in the `/packages/` folder. Unzip the folder in a separate location on your computer because it will need to built as its own Qt project. You can then follow the same instructions to build this library that Stanford normally provides on their course websites [HERE](https://web.stanford.edu/dept/cs_edu/resources/qt/). 

Instead of building their new libraries, however, you will just build this older version. It will install all the necessary Stanford specific libraries on your system for use with Qt.

## Overview

In October of 2022, Donald Knuth released *The Art of Computer Programming: Volume 4b: Combinatorial Algorithms, Part 2*. In this work he revised his previous implementation of his Algorithm X via Dancing Links. His revision included changing the doubly linked links that solved backtracking problems to an array of nodes. This is an interesting optimization. It provides good locality, ease of debugging, and memory safety if you use an array that does not need to be managed like a C++ vector. One of the points Knuth makes clear through his section on Dancing Links is that he wants to give people the tools they need to expand and try his backtracking strategies on many problems. If you want the original Algorithm, please read Knuth's work. Below, I will discuss how I applied or modified his strategies to the problems in this repository. I solve three problems: supplying a transportation grid, perfect matching for partners, and maximum weight matching for partners.


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

Here is the struct for the type we place in the lookup table. This will manage our recursion. When the lookup table head points to itself, we know all cities are covered.

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

I then place vectors of these types as private members of my class. The `numItemsAndOptions` is just the number of cities we must cover. There is an equivalent number of cities as items to cover and cities as supply options.

```c++
std::vector<cityHeader> table_;
std::vector<cityItem> grid_;
int numItemsAndOptions_;
```

Here is a literal representation of the array and how it appears in memory for the example I previously illustrated.

The lookup table that controls recursion and helps us collect supply location names is a separate array.

![lookup-table-illustrated](/images/lookup-table-illustrated.png)

The core logic of the algorithm takes place in this array where items exist in all of the sets they are connected to.

![matrix-array-illustrated](/images/matrix-array-illustrated.png)

There are some leftovers from Knuth's logic that I am not sure what to do with in this problem and don't play a role to solve the supply problem. Compare this image to the illustration in Knuth's work and the table he produces of his array to see more differences.

Unfortunately, I cannot take advantage of the implicit left right nature of array elements next to one another in this implementation. I need explicit `left` `right` fields because I remove cities from all other options that they appear in without eliminating the other options for future supply possibilities. This means I must splice them out of a doubly linked list.

## Supply Tags: A Second Approach

The previous approach is unable to take advantage of the fact that cities in a row are directly next to one another in an array. I thought that using a quadruple linked list--up, down, left, and right--was required. This way I could cut a city out of the world while keeping other supply options available. This is actually not necessary.

I can minimize the work being done by simply tagging all the cities covered by the supply option with a unique tag. Luckily this problem supplies a unique tag: the number of supplies. The number of supplies we are given is the limit to the depth of our recursion. So, if we use the number of supplies we have left as a unique tag for the cities that are covered, we can avoid much complexity. Take for example the same map we worked with previously.

![grid-1](/images/grid-1.png)

We are given two supplies to cover this grid, so we will use them to tag the cities we supply.

![supply-tag](/images/supply-tag.png)

Here are the key details of the above image:

- We always tag the cities in the supply option with same unique tag as the header for the cities covered by that location.
- When we enter the next level of recursion we have a new number of supplies as we try to cover E and F.
- We tag all cities in option B with the same number.
- When we need to cleanup after distributing these two supplies, we will know that E and F were the only cities covered by supplying city B. Cities A, B, C, and D were already covered. We know this because the supply tags do not match.
- We do not need to traverse the up-down linked list once we tag all cities in an option. It is sufficient to tag the cities in the supply option we chose and the headers for those cities.

For this small example that has a successful supply scheme with two supplies it might not be clear why this is so useful. However, for large maps it becomes important to know which cities to uncover if a supply location does not work out. While backtracking, these tags help only uncover those cities that were covered when we entered a specific level of recursion.

An added benefit of this approach is that the types are much simpler and we eliminate any traversals of the up-down linked list, other than to try every supply option that covers a city. This implementation is faster than the previous implementation. However, the speed gain only becomes noticeable on larger transportations grids.

### Supply Tags Implementation

Much of the implementation is the same. Here is the new type that is able to get rid of the left-right field in the dancing links grid.

```c++
typedef struct city {
    int topOrLen;
    int up;
    int down;
    // New addition! supplyTag instead of left-right.
    int supplyTag;
}city;
```

The lookup table with the city names is identical.

```c++
typedef struct cityName {
    std::string name;
    int left;
    int right;
}cityName;
```

The private members of the class remain the same.

```c++
std::vector<cityName> table_;
std::vector<city> grid_;
int numItemsAndOptions_;
```

The dancing links grid will look slightly different without the left right field.

![supply-tag-array-illustrated](/images/supply-tag-array-illustrated.png)

There are some new details from the above image.

- We use the spacer nodes, the ones with the negative `topOrLen` field, to tell us the supply option and how to continue iterating through the cities in a supply option to cover.
- The spacer nodes point up to the first element in the previous option and down to the last element in the current option. This helps us visit and tag all cities in an option regardless of which city we start at.
- The supply tag is `0` by default, telling us a city needs to be covered. It will then take the tag of whatever supply number it is given when covered.

Unfortunately, this implementation is still not fast enough to crack the code on the U.S. Map. It can confirm that the U.S. can be covered with 30 supplies, slightly faster than the other implementation, but gets stuck trying to find the optimal number between 20 and 30 supplies. The usage instructions are the same for both solvers and follows below.

## Disaster Planning Usage Instructions

I chose to implement the solution to these problems as classes that can be found in the **[`DisasterLinks.h`](/dancing-links/dlx/DisasterLinks.h)**/**[`DisasterLinks.cpp`](/dancing-links/dlx/DisasterLinks.cpp)** and **[`DisasterTags.h`](/dancing-links/dlx/DisasterTags.h)**/**[`DisasterTags.cpp`](/dancing-links/dlx/DisasterTags.cpp)** files, respectively. There are tests at the bottom of the all `.cpp` files that make the internals of the dancing links data structure more understandable.

I then included these implementations in the **[`DisasterGUI.cpp`](/dancing-links/dlx/Demos/DisasterGUI.cpp)** file as the solvers for the maps included in this repository. If you want to see how each implementation solves the provided maps use the following steps.

1. Open the project in Qt Creator with the correct Stanford C++ library installed. (See the [Build Note](#build-note)).
2. Build and run the project.
3. Select the `Disaster Planning` option from the top menu.
4. Select any map from the drop down menu at the bottom of the window.
5. Select the implementation that you want to solve the map in the drop down menu: a traditional set based implementation, a quadruple linked dancing links solver, or the supply tag implementation. You can distinguish the solvers by the colors they paint the cities.
6. Press `Solve` to find the first Optimal Map coverage found or press `All Optimal Solutions` to find all possible supply distributions with the optimal number of supplies. You can cycle through configurations with the next and previous arrows if you find all solutions. 

It is interesting to note which cities have more flexibility with their disaster supplies. For example, Colorado is a slightly larger map than Tokyo. However, all solvers take a noticeable amount of time to tell us that Colorado has only 136 configurations with optimal supplies. Then, when asked about Tokyo, all solvers will quickly report that there are 790 viable configurations for the optimal number of supplies. Some maps are more challenging than others but the challenge may not be directly related to the number of configurations that are possible to generate.

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

- Every person appears as an item along the top headers.
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

I am not currently aware of any optimizations for this problem. It is very fast at finding solutions, especially because it only must find one. I added the bonus functionality of finding all possible perfect matching configurations to the graph editor application. To solve that problem, the algorithm requires minimal adjustments and it is remarkably fast. For example, requesting all possible perfect matchings on `Sample3`, a somewhat large network of connections, will yield 630 results immediately. By adding nodes, you can eventually begin generating configurations in the 10 to 100's of thousands in a reasonable amount of time. In, contrast max weight matching is much more difficult.

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
- When we cover an individual person, the only options that disappear are the pairings the chosen person can affect by being absent.
- We do NOT remove all options that include this person's partner. For example, we do not remove all other options that have B or C as a possible partner.
- F becomes unreachable and that is completely acceptable. In fact, we will continue to attempt combinations of partners until the world is empty or nobody can be paired.

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

I place the two arrays and some extra logic as private members in my class.

```c++
std::vector<personName> table_;
std::vector<personLink> links_;
int numPeople_;      // Total people in the network.
int numPairings_;    // The number of pairings or rows in the matrix.
bool hasSingleton_;  // No perfect matching if someone is alone.
bool isWeighted_;    // Must provide weights for max weight matching.
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
5. Select the solver that you want to use for these problems. I have included a solver based on the implementation discussed above and a more traditional implementation that uses sets. Finally, I included a solver that uses the *Blossom Algorithm* written by Ed Rothberg. This can solve Perfect Matching and Max Weight Matching but Rothberg does not solve the All Perfect Matchings problem.
6. Press `Find Perfect Matching`, `Find All Perfect Matchings` or `Find Max-Weight Matching` to solve the appropriate problem.

While I think Knuth's dancing links are well suited to Perfect Matching and All Perfect Matchings, I don't think they are the best fit for the Weighted Matchings Problem, at least not as I have currently implemented the solution. To see why, try the DLX solver and Rothberg's solver on the `MaxWeightStress` file. The dancing links solver for that problem is just a creative, efficient approach to a brute-force recursive algorithm. Rothberg's solution, on the other hand, is extremely fast. I will try to explore ways to speed things up.

## Bonus: Pokémon Type Coverage 

Knuth brings up many puzzles in his work on Combinatorial Algorithms. He has word puzzles, number puzzles, visual *puzzle* puzzles. All of them got me thinking about more problems I could find that use his methods and I found one! The videogame Pokémon by developer Game Freak is part of a well established franchise that started in the 1990's. At their core, the Pokémon videogames are a game of rock-paper-scissors with a few dozen layers of complexity added on top. While I can't explain the entire ruleset of Pokémon here, I can explain the relevant parts to the problem I decided to solve.

In Pokémon, trainable animals/monsters battle on your behalf imbued with pseudo-elemental types like fire, water, flying, ground, and many others. There are 15 to 18 fundamental types that Pokémon can use depending on the generation of games you are considering. Because Pokémon has been around for so many years, they have made many changes to their games. The community that plays those games often divides the games by generations in which different game mechanics were introduced. Right now, there are nine generations. At the most abstract level you only need to consider how to attack other Pokémon with these fundamental types and how to defend against these attack types with your own Pokémon. The twist on defense is that your Pokémon can have up to two different types--such as Fire-Flying or Ice-Water--that determine how weak or resistant they are to these fundamental attack types. While these fundamental types could be combined to form 306 unique dual types, not to mention the additional 15 to 18 single types, the developers of this game have not done so yet. Depending on the generation, there are far fewer types than this, but there are still many. The most recent release is up to 162 unique Pokémon types.

So, there are two cover problems hiding in the complexities of the Pokémon games: one for defense and one for attack. The two essential cover questions we can ask are as follows.

- Which teams of at most 6 Pokémon--the most you can carry with you at once--give me resistance to every attack type I will encounter? If you consider an entire game, you would want to know the answer to that question for every attack type in the game. If you are considering just the attacks you will face in some portions of the game, then the range of attack type shrinks but the question remains the same.
- Which attack types can I choose to be effective against every defensive type I will encounter in the game? Again, considering the entire game versus some smaller sections will change the range of defensive types you will see, but the question remains the same.

To try to answer these questions we can begin to organize Pokémon's data with Knuth's dancing links method. 

### Defense

Let's start with the defensive case. Here is a small subset of both attack and defensive types we can look at as a toy example.

![defense-links](/images/defense-links.png)

Here are the key details from the above image.

- Attack types we need to cover are our items and defensive types we can use to resist those attack types are our options.
- There are different levels of resistance that types can have to attack types. You can think of resistances as fractional multipliers that are applied to what would otherwise be normal damage. The best we can do is nullify damage by being immune to that type. You can see this in the cases of Ground-Water vs Electric and Bug-Ghost vs Normal type attacks. A `x0.25` multiplier is the next best followed by a `x0.50`.
- While single defense types exist, I am not including them here to avoid confusion.
- The order of the two types in a dual type does not change any resistance or weakness to attack types. I organize all dual-types alphabetically for consistency.

To solve an **exact cover** version of the defensive problem, as Knuth describes in his work, we want to resist each of these attack types exactly once across the options we choose. You can take a moment to try to pick the types that achieve this before we go over the selection process.

![defense-links-2](/images/defense-links-2.png)

Here are the key details from the above image.

- We start by trying to cover an attack type that appears the least across all options, Normal. We choose option `Bug-Ghost`.
- This choice eliminates `Bug-Ghost` and any other options that contain the attack items `Grass` and `Normal` because those items have already been covered by our current choice. 

We are then left with a shrunken grid to consider solving.

![shrink-defense-1](/images/shrink-defense-1.png)

If we follow the same selection and elimination process we are led to the following solution.

![solve-defense](/images/solve-defense.png)

Here are the key details from the above image.

- Our remaining two selections are marked with the red asterisk.
- We know we have solved the problem when no items remain in the world.
- The heuristic of selecting an item that appears few times across all options will often decrease the number of iterations of this algorithm.

While it is good to see how quickly this process shrinks the problem, it can sometimes be hard to see what about this selection process makes it an **exact cover**. To help visualize the exact nature of these choices here is another way to look at the cover solution.

![exact-walk-defense](/images/exact-walk-defense.png)

Here are the key details from the above image.

- No two defensive types that we chose covered the same attack type. I find it easier to think of this as a perfect walk across the items from left to right with no wasted steps, as illustrated by the line I take through these items.
- In addition to finding all solutions to this problem, I rank the solutions that I find according to a scoring system for defense.
  - The `x0.00` multiplier is `1 point`.
  - The `x0.25` multiplier is `2 points`.
  - The `x0.50` multiplier is `3 points`.
  - A **LOWER** score is better than a higher score. This favors stronger resistances.
- This scoring systems is an arbitrary product of my implementation based on what I assume is good in these games. I haven't thought critically about Pokémon in quite some time so maybe an expert could provide more guidance here.

All choices considered, here is the final score of our **exact cover** team of 3.

![defensive-cover](/images/defensive-cover.png)

In such a small case the significance of this score may not be apparent, but when generating thousands of solutions it can become helpful to sift through them with a ranking system. Let's now look at Attack.

### Attack

We will use the same types we did for the defensive example, but flip attack and defense.

![attack-links](/images/attack-links.png)

Here are the key details from the above image.

- Defensive types are the items we must cover with attack types that serve as our options.
- We can only have single attack types. No attack in the game does two simultaneous forms of damage.
- `Normal` is not effective against any types in this case. This is possible for many types depending on the size of the subproblem and simply means we will not choose it as an option. Fun fact: Normal is the only attack type that receives no damage multiplier against other types in the game.

The shrinking of this problem is exactly the same as in the defensive case--choose the item that appears the least across all options and try to cover it first--so instead I will show the **exact cover** walk across the items that forms the solution.

![exact-walk-attack](/images/exact-walk-attack.png)

Here are the key details from the above image.

- No two attack types are effective against the same defensive type. Just complete the walk from left to right to see this.
- In addition to choosing our attack types, we can rank them as we choose under the following point system.
  - The `x2` multiplier is `5 points`.
  - The `x4` multiplier is `6 points`.
  - A **HIGHER** score is better. This favors stronger attack types for the given defensive items.
- I use this scoring method because I assume quadruple damage is more desirable and a one point difference seems fair if we are only using it to distinguish the quality of the solutions we find.

Again, this ranking system will help when we generate many solutions. Here is the rank of our final picks.

![attack-cover](/images/attack-cover.png)

Now that I have summarized how **exact cover** works for the Pokémon type coverage problem, we can briefly discuss **overlapping coverage**.

### Overlapping Coverage

An **exact cover** can be difficult to achieve. Depending on the Pokémon generation, it can be impossible. In addition, most people don't think about being so exact with their choices so that no choice is wasted. Rather, they might just take a sweeping approach, trying to get as much coverage as possible. So, it can be fun to look at cover in a slightly different way, allowing for overlap. With a simple adjustment we can find many more solutions to the cover problem.

![overlapping-defense](/images/overlapping-defense.png)

Here are the key details from the above image.

- We select the same starting option `Bug-Ghost`.
- When we cover the items in those options we leave all other options that contain those items available to solve the cover problem.

By allowing other options to remain available we end up with a slightly different walk from left to right to solve the problem.

![overlapping-walk-defense](/images/overlapping-walk-defense.png)

Here are the key details from the above image.

- All scoring systems remain the same as in previous examples.
- Three of our choices for options overlap on the `Grass` coverage. This is acceptable because our goal is to simply cover all attack types within our 6 choice limit.

Here are the results of those choices.

![overlapping-defense-cover](/images/defense-overlapping-cover.png)

There are a few other **exact** and **overlapping** covers within this example if you want to try finding your own to solidify the concepts. You could also try the Attack version, which operates under the exact same principles.

### Pokémon Planning Implementation

In order to accomplish the in-place, no-copy recursion that comes with Knuth's Dancing Links, I have chosen to use a C++ vector. In older implementations of Dancing Links, Knuth used a 4-way linked grid of nodes with up, down, left, and right fields. Now, the left-right fields of these nodes can be implicit because we place everything in one vector.

Here is the type that I use to manage the recursion and know when every item is covered. The name corresponds to the item.

```c++
typedef struct typeName {
    std::string name;
    int left;
    int right;
}typeName;
```

Here is the type that I use within the dancing links array.



### Pokémon Planning Usage Instructions

I have created a small testing ground for the Pokémon Cover Problem. I adapted a graph drawing application written by Keith Schwarz and Stanford course staff to allow you to explore various Pokémon maps. The maps are divided by Pokémon Generation. For example, the Kanto map is based around the attack and defense types available in Generation I of Pokémon. I included a Generation V and Generation IX map as well with more types available as you progress through generations.

You can solve the maps entirely for exact and overlapping cover problems or you can select specific gyms. In Pokémon, you progress through the game by defeating 8 gym leaders and then a final group called the Elite 4 (along with one last champion of that league). You can select any combination of gyms to defend against or attack and the cover problem will be adapted to the types in those locations. Interesting results can arise as you plan out your type advantages!

1. Open the project in Qt Creator with the correct Stanford C++ library installed. (See the [Build Note](#build-note)).
2. Build and run the project.
3. Select the `Pokémon Planning` option from the top menu.
4. Solve for every possible type you can encounter on a map with the cover buttons.
5. Select only specific gyms that you would like to cover with the `G1`-`E4` buttons.
6. Clear all selections at any time with the `CL` button.

I find it interesting that only Generation IX, the `Paldea.dst` map, has an exact cover for all possible types you will encounter in that generation. I am no expert on game design, but perhaps that communicates the variety and balance that Game Freak has achieved in their later games. However, looking at smaller subsets of gyms in the other maps can still plenty of fun!

## Citations

This project grew more than I thought it would. I was able to bring in some great tools to help me explore these algorithms. So, it is important to note what I am responsible for in this repository and what I am not. The code that I wrote is contained in the following files.

- `DisasterLinks.h`
- `DisasterLinks.cpp`
- `DisasterPlanning.cpp`
- `DisasterTags.h`
- `DisasterTags.cpp`
- `Matchmaker.cpp`
- `PartnerLinks.h`
- `PartnerLinks.cpp`
- `PokemonLinks.h`
- `PokemonLinks.cpp`
- `PokemonParser.cpp`

As mentioned in the intro, the core ideas of Algorithm X via Dancing Links belongs to Knuth, I just implemented it a few different ways.

Any other code was rearranged or modified to fit the needs of this project. It might have been starter code provided by Stanford course staff, or applications they wrote. All other work was originally provided by Keith Schwarz and Stanford Course Staff as components of the original assignment that inspired this project.

I made the most significant modifications to the following files to allow me to use different solver algorithms for the problems and change how they displayed to the GUI.

- `DisasterGUI.cpp`
- `MatchmakerGUI.cpp`
- `PokemonGUI.cpp`

However, Keith Schwarz and Stanford course staff are completely responsible for making those usable applications in the first place. Finally all files in the `FastMatching` folder are written by Ed Rothberg and ported to C++ by Keith Schwarz. These were included out of my own curiosity to see how a fast solution to the `Max Weight Matching` problem would feel to use in the application GUI.
