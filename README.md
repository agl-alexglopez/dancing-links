# Dancing Links

Donald Knuth recently released *The Art of Computer Programming: Volume 4b: Combinatorial Algorithms, Part 2*. In this work he revised his previous implementation of his Algorithm X via Dancing Links. His revision included changing the doubly linked links that solved backtracking problems to an array of nodes. This is an interesting optimization. It provides good locality, ease of debugging, and memory safety if you use an array that does not need to be managed like a C++ vector. One of the points Knuth makes clear through his section on Dancing Links is that he wants to give people the tools they need to expand and try his backtracking strategies on many problems. If you want the original Algorithm, please read Knuth's work. Below, I will discuss how I applied or modified his strategies to these problems.


## Dancing with Danger

Let's look at transportation grids. Take the following grid as an example.

![grid-1](/images/grid-1.png)

We are given a certain number of supplies with which we can cover this grid. Let's start with 2 supplies. Can we cover this grid? A city is covered under the following conditions.

- A city has supplies.
- A city is adjacent to a city that has supplies.
- A city may only be supplied once.
- If a city is adjacent to a city with supplies, thus making it covered, it may still receive supplies to help other cities.

The answer in this grid is fairly obvious. Supply cities D and B. However, how can we use Knuth's methods to represent this grid and solve the problem?

### The Grid

