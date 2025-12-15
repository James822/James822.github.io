+++
date = '2025-03-09'
draft = false
title = 'Introduction to Group Theory Part 1: Permutations'
+++

{{< katex >}}

I've recently taken a course in group theory, a branch of abstract algebra that studies the mathematical structures known as **groups**.

Abstract algebra is a fascinating study in mathematics, as it unifies concepts previously scattered in different areas. Boolean algbera for example is the mathematical structure that underlies working with operations on any kind of binary system. Propositional logic, digital circuit design (logic gates), and even set operations are all forms of boolean algebra. Set operations, propositonal logic, and the design of combinational circuits may seem disparate; but their structure is essentially the same.

This is why I have come to love abstract algebra as a discpline. At it's core, it is the formal study of mathematical **essence**. Two structures may have different **substance**, but if they function exactly the same up to some kind of isomorphism; then they are the *essentially* the same.

Groups, the primary study of *group theory*, happen to be a type of structure that is encountered very frequently in mathematics. Many other structures such as vector spaces, rings, fields, the real numbers; can all be shown be groups. Therefore, studying groups will reveal properties shared with all of these other structures.

Group theory is quite a vast topic, so for this part I'm only going to focus on the study of permutations in this post, but this is not without justification. According to [Cayley's Theorem](https://en.wikipedia.org/wiki/Cayley's_theorem), every group is isomorphic to some permutation group. This is another way of saying all groups can be understood in terms of permutations, so it might not be such a bad idea to start there. In the next part we will go over how permutations relate to groups more generally.


## Permutations
A **permutation** of \(n\) numbers is a bijection from \(\{1,2,...,n\}\) to itself. So it is a surjective function of the form:

\[ \tau : \{1,2,...,n\} \to \{1,2,...,n\} \]

As an example, let us define \(\tau\) with \(n=3\) to be the following function:

\[ \tau : \{1,2,3\} \to \{1,2,3\} \]
Given by the rule: \[ \tau (1) = 2, \tau (2) = 3, \tau (3) = 1 \]

This means simply that 1 is sent to 2, 2 is sent to 3, and 3 is sent to 1.

Rather than painstakingly write out the function value for each input, we can represent this same exact function \(\tau\) in tabular form as the following:

\[
\begin{bmatrix}
1 & 2 & 3\\
2 & 3 & 1
\end{bmatrix}
\]

This format is known as [Cauchy's](https://en.wikipedia.org/wiki/Augustin-Louis_Cauchy) two-line notation, but we will refer to it as tabular form. All entries are read "top-down" per column. In any given column, the element of the top row is "sent" to the element of the bottom row.

More generally, we can represent any permutation with the following form:

\[
\begin{bmatrix}
1 & 2 & 3 & \cdots & n\\
a_1 & a_2 & a_3 & \cdots & a_n
\end{bmatrix}
\]

And this is read as 1 goes to \(a_1\), 2 goes to \(a_3\), 3 goes to \(a_3\),..., \(n\) goes to \(a_n\). Here, \(n\) is the number of elements to be permuted, and \(a_i\) are the numbers belonging to the set \(\{1,2,...,n\}\), and it also must be the case that \(a_i \neq a_j\) iff \(i \neq j\). Basically, no \(a_i\) can appear more than once. Any kind of table that follows this specific format is a valid permutation.

Notice also that the order of columns does not matter, as we could freely swap them to our liking without changing the permutation. As a matter of style, we typically like to order the columns by the first/top row with 1,2,3,... etc.

Every finite collection of \(n\) objects can be *indexed*, that is to say, we can label every object with a number from 1 to \(n\). Because of this simple and obvious fact, it is sufficient to only study permutations with the set \(\{1,2,...,n\}\), as every finite set can be put into a one-to-one correspodence with this set.


## Compositions of Permutations
Since permutations are bijective functions from a set to itself, it is possible to compose any number of permutations we like, as long as the permutations are both of \(n\) elements.

The operation of function composition is given by \(\sigma \circ \tau (a) = \sigma (\tau (a))\) for two permutations \(\tau\) and \(\sigma\). This means we apply the permutation \(\tau\) first, and then we apply \(\sigma\) second. Composition is read from "right-to-left" which can be confusing.

For example, take two permutations such as these:

\[
\tau = 
\begin{bmatrix}
1 & 2 & 3\\
2 & 1 & 3
\end{bmatrix}
,
\sigma = 
\begin{bmatrix}
1 & 2 & 3\\
2 & 3 & 1
\end{bmatrix}
\]

Let's see where the number "1" goes under the composition of \(\sigma \circ \tau\): \(\tau (1) = 2\), which implies that \(\sigma (\tau (1)) = \sigma(2)\), and \(\sigma (2) = 3\), so that implies that \(\sigma \circ \tau (1) = \sigma (\tau (1)) = 3\). A similar process can be performed for 2 and 3 to show that: \(\sigma (\tau (2)) = 2\) and \(\sigma (\tau (3)) = 1\)

There is a much easier way to create compositions of permutations by using the tabular form directly. Using the previous example, Take the first permutation \(\tau\) and superimpose it on top of the second permutation \(\sigma\). (order matters here, don't put \(\sigma\) on the top and \(\tau\) on the bottom). This will create a matrix that has 4 rows, and it should look like this:

\[
\begin{bmatrix}
1 & 2 & 3\\
2 & 1 & 3\\
-&-&-\\
1 & 2 & 3\\
2 & 3 & 1
\end{bmatrix}
\]

Then, rearrange the columns of the bottom matrix \(\sigma\) so that the numbers at the top row of \(\sigma\) match the numbers of the bottom row of \(\tau\). In this example, all we need to do is swap column 1 and column 2 of \(\sigma\) just like so:

\[
\begin{bmatrix}
1 & 2 & 3\\
2 & 1 & 3\\
-&-&-\\
2 & 1 & 3\\
3 & 2 & 1
\end{bmatrix}
\]

The next step is to simply remove the bottom row of \(\tau\) and remove the top row of \(\sigma\). Basically, remove the middle two rows of the whole matrix and leave the rest:

\[
\begin{bmatrix}
1 & 2 & 3\\
3 & 2 & 1
\end{bmatrix}
\]

But wait, doesn't this just look like another permutation? And in fact it is! Compositions of permutations are themselves permutations! This may seem obvious, but it's an important result that we'll come back to later.

Compositions of permutations are also **associative** meaning that for any three permutations \(\tau\), \(\sigma\), and \(\rho\):

\[ \rho \circ (\sigma \circ \tau) = (\rho \circ \sigma) \circ \tau \]

Associativity follows from the fact that function composition in general is always associative, but it is NOT the case that compositions of permutations are always **commutative**. In general:

\[ \sigma \circ \tau \neq \tau \circ \sigma \]

This inequality doesn't hold for every permutation, as there are some permutations that do commute. We will see some examples of commutative permutations in the next section when we talk about disjoint cycles.


## Cycle notation
Another way to represent permutations is with something called **cycle notation**.

A **cycle** is a permutation that takes a collection of *distinct* numbers in a cycle, leaving all other numbers fixed. For example: 4 -> 2, 2 -> 6, 6 -> 3, 3 -> 4; is a valid cycle because it wraps back around and no number is visited more than once (besides of course the starting number 4, which must be visited twice). The other numbers are left fixed and unchanged.

We can represent this example with the following cycle notation:

\[
\begin{pmatrix}
4 & 2 & 6 & 3
\end{pmatrix}
\]

Each number sends you to the next one, and once you reach the end of the list, it wraps back around to the first element. This example is a 4-cycle as there are 4 distinct numbers in the cycle. It also doesn't matter where we start the list as long as we maintain the order, so we could also represent the same 4-cycle as so:

\[
\begin{pmatrix}
2 & 6 & 3 & 4
\end{pmatrix}
\]

and you can verify that this changes nothing, but we tend to prefer to start cycles with the least element as a matter of style. Since any k-cycle is also a permutation, we can also represent it using our tabular form like before. The numbers 1 and 5 are not in this cycle which means that they stay fixed as we mentioned before, so we can write:

$$
	\begin{pmatrix}
	2 & 6 & 3 & 4
	\end{pmatrix}
	=
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5 & 6\\
	1 & 6 & 4 & 2 & 5 & 3
	\end{bmatrix}
$$

But can *any* permutation be represented by a k-cycle? Let's investigate this to see if it is true by considering the following permutation:

$$
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5\\
	2 & 1 & 4 & 5 & 3
	\end{bmatrix}
$$
  
Could we represent this as a single cycle? The answer is no! You can try as hard as you want, but there won't be a single k-cycle that can represent this permutation. We can however represent this as a composition of 2 cycles like this:

$$
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5\\
	2 & 1 & 4 & 5 & 3
	\end{bmatrix}
	=
	\begin{pmatrix}
	1 & 2
	\end{pmatrix}
	\circ
	\begin{pmatrix}
	3 & 4 & 5
	\end{pmatrix}
$$

Cycle composition follows the exact same rule as permutation composition like we discussed before. Don't forget that a cycle is a permutation, just a different way of representing it.

Rather than using the \(\circ\) notation, we typically just leave it out and only write:

$$
	\begin{pmatrix}
	1 & 2
	\end{pmatrix}
	\begin{pmatrix}
	3 & 4 & 5
	\end{pmatrix}
$$

with the understanding that there is an implicit composition between the two cycles, and order also matters as it goes from right-to-left. If we have more than two compositions, we also don't need to use brackets because we know that compositions of permutations are associative.

It can be proved that every permutation can be represented either as a single cycle, or as a composition of 2 or more *disjoint* cycles. It is also the case that the order of compositions doesn't matter with disjoint cycles, so the following equality holds for the previous example:

$$
	\begin{pmatrix}
	1 & 2
	\end{pmatrix}
	\begin{pmatrix}
	3 & 4 & 5
	\end{pmatrix}
	=
	\begin{pmatrix}
	3 & 4 & 5
	\end{pmatrix}
	\begin{pmatrix}
	1 & 2
	\end{pmatrix}
$$

The idea is that if a composition of any number of cycles are disjoint, then there are no elements in common. Any element will only be "permuted" by exactly one of the cycles that happens to contain that element. The other cycles will leave that element fixed, so therefore it doesn't really matter where we place the cycle in the composition.

Therefore, any composition of disjoint cycles is **commutative**.

Also, if we wanted to, we could for example rewrite the cycle \(\begin{pmatrix}3 & 7 & 5\end{pmatrix}\) as:

$$
	\begin{pmatrix}
	1
	\end{pmatrix}
	\begin{pmatrix}
	2
	\end{pmatrix}
	\begin{pmatrix}
	3 & 7 & 5
	\end{pmatrix}
	\begin{pmatrix}
	4
	\end{pmatrix}
	\begin{pmatrix}
	6
	\end{pmatrix}
$$

but writing all of these extra 1-cycles is redundant, so we usually leave them out.

If we want to write the *identity* permutation (that is, the permutation that takes every number to itself), then we can simply write a cycle with empty brackets \(\begin{pmatrix}\end{pmatrix}\). This element is special, and so we give it the letter \(e\) (not to be confused with euler's number):

$$
	e
	=
	\begin{pmatrix}
	\end{pmatrix}
	=
	\begin{bmatrix}
	1 & 2 & 3 & \cdots & n\\
	1 & 2 & 3 & \cdots & n
	\end{bmatrix}
$$

with the special property that for any permutation \(\sigma\):

$$
	\sigma \circ e = e \circ \sigma = \sigma
$$


## Converting Between Cycle Form and Tabular Form
There is a straightforward way to convert from cycle-form to tabular-form so let's take an example composition of disjoint cycles and see how we can convert it to tabular form:

$$
	\begin{pmatrix}
	1 & 5
	\end{pmatrix}
	\begin{pmatrix}
	2 & 4 & 6
	\end{pmatrix}
	\begin{pmatrix}
	7 & 9
	\end{pmatrix}
$$

Since the largest number here is 9, that means that we are dealing with 9 numbers to be permuted, so create an empty table that has numbers 1 through 9 on the top, and leave the bottom row empty:

$$
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5 & 6 & 7 & 8 & 9\\
	. & . & . & . & . & . & . & . & .
	\end{bmatrix}
$$

Our goal here is to "fill in the dots" and determine what numbers need to go there. For starters, we observe that the numbers 3 and 8 are not in the cycles whatsoever, so we know that they stay fixed:

$$
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5 & 6 & 7 & 8 & 9\\
	. & . & 3 & . & . & . & . & 8 & .
	\end{bmatrix}
$$

Then, we start at the number 1 and see where it maps to. Since the only cycle that 1 belongs to is: \( \begin{pmatrix}1 & 5\end{pmatrix} \), we know that 1 maps to 5:

$$
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5 & 6 & 7 & 8 & 9\\
	5 & . & 3 & . & . & . & . & 8 & .
	\end{bmatrix}
$$

Similarly, we can see that 2 maps to 4 because of the cycle \( \begin{pmatrix}2 & 4 & 6\end{pmatrix} \):

$$
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5 & 6 & 7 & 8 & 9\\
	5 & 4 & 3 & . & . & . & . & 8 & .
	\end{bmatrix}
$$

And if we continue this process we get for the remaining numbers, then we see that we get the following permutation in tabular form:

$$
	\begin{pmatrix}
	1 & 5
	\end{pmatrix}
	\begin{pmatrix}
	2 & 4 & 6
	\end{pmatrix}
	\begin{pmatrix}
	7 & 9
	\end{pmatrix}
	=
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5 & 6 & 7 & 8 & 9\\
	5 & 4 & 3 & 6 & 1 & 2 & 9 & 8 & 7
	\end{bmatrix}
$$

and you can verify that this is correct.

We also can do the opposite and convert from tabular form to disjoint cycle-form by simply picking numbers and seeing what cycles they belong to. Let's take this same example and convert it back to cycle-form. To start, we pick the number 1 and "follow" it until we get back to 1. In this case, 1 goes to 5, and 5 goes back to 1, so we write that cycle out: 

$$
	\begin{pmatrix}
	1 & 5
	\end{pmatrix}
$$

Then, we pick the next number that has not been visited yet. So we pick 2, and see where that goes. 2 goes to 4, 4 goes to 6, 6 goes to 2; so that should give us \(  \begin{pmatrix}2 & 4 & 6\end{pmatrix}\). Now add this cycle to the existing cycles like so: 

$$
	\begin{pmatrix}
	1 & 5
	\end{pmatrix}
	\begin{pmatrix}
	2 & 4 & 6
	\end{pmatrix}
$$

And then we move to the next number that is not already in a cycle, which is 3; and we see that 3 goes to itself, so we can leave it out.

Then the next number that we haven't visited yet is 7. If we follow 7 then we see that 7 goes to 9 and then goes back to 7, giving us:

$$
	\begin{pmatrix}
	1 & 5
	\end{pmatrix}
	\begin{pmatrix}
	2 & 4 & 6
	\end{pmatrix}
	\begin{pmatrix}
	7 & 9
	\end{pmatrix}
$$

Finally, the last number we need to visit is 8, but 8 just maps back to itself so we leave it as is.

Using cycle notation is generally preferred over tabular form as it takes up less space, and it also represents a core feature of the permutation in a better way. Using disjoint cycles is really handy so we will continue with this format primarly, but feel free to use tabular form as well to help you understand cycle form. Note also that compositions of non-disjoint cycles is perfectly valid too, but we prefer to use disjoint cycles whenever possible.


## Inverses and the Identity Permutation
Every permutation has an inverse permutation. For every permutation \(\sigma\), there exists an identity permutation which we denote \(\sigma^{-1}\), such that the following holds:

$$
	\sigma^{-1} \circ \sigma = \sigma \circ \sigma^{-1} = e
$$

The proof that every permutation has an inverse follows from the fact that permutations are bijective functions from a set to itself. Every bijective function has an inverse, and since permutations are from a set to itself; then that inverse is also a permutation. The inverse is also *unique*, but we'll not prove that.

Let's take the following permutation \(\sigma\) as an example:

$$
	\begin{pmatrix}
	1 & 4
	\end{pmatrix}
	\begin{pmatrix}
	3 & 7 & 5
	\end{pmatrix}
$$

The inverse of this permutation is \( \begin{pmatrix}1 & 4\end{pmatrix}\begin{pmatrix}3 & 5 & 7\end{pmatrix} \) as:

$$
	\begin{pmatrix}
	1 & 4
	\end{pmatrix}
	\begin{pmatrix}
	3 & 7 & 5
	\end{pmatrix}
	\begin{pmatrix}
	1 & 4
	\end{pmatrix}
	\begin{pmatrix}
	3 & 5 & 7
	\end{pmatrix}
	=
	e
$$

Recall that cycle composition is from right-to-left. Take every number from 1 to 7 and move it from right to left across these 4 cycles, and you will see that the number ends up back at itself.

For example: 1 goes to 4 in \( \begin{pmatrix}1 & 4\end{pmatrix} \), 4 stays fixed in \( \begin{pmatrix}3 & 7 & 5\end{pmatrix} \), 4 goes to 1 in \( \begin{pmatrix}1 & 4\end{pmatrix} \), and 1 stays fixed in \( \begin{pmatrix}3 & 5 & 7\end{pmatrix} \). So, 1 goes to 1, itself. The same is true for every other number.

If we go back to using tabular form, then there is similar "trick" to finding out what the inverse of any permutation is. First, let's take this same example and convert it to tabular form:

$$
	\begin{pmatrix}
	1 & 4
	\end{pmatrix}
	\begin{pmatrix}
	3 & 7 & 5
	\end{pmatrix}
	=
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5 & 6 & 7\\
	4 & 2 & 7 & 1 & 3 & 6 & 5
	\end{bmatrix}
$$

To find the inverse in tabular form, simply swap the top row and the bottom row:

$$
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5 & 6 & 7\\
	4 & 2 & 7 & 1 & 3 & 6 & 5
	\end{bmatrix}^{-1}
	=
	\begin{bmatrix}
	4 & 2 & 7 & 1 & 3 & 6 & 5\\
	1 & 2 & 3 & 4 & 5 & 6 & 7
	\end{bmatrix}
$$

If you want, you can then swap the columns around so that the top row is ordered from 1,2,3,... and so on, just like this:

$$
	\begin{bmatrix}
	4 & 2 & 7 & 1 & 3 & 6 & 5\\
	1 & 2 & 3 & 4 & 5 & 6 & 7
	\end{bmatrix}
	=
	\begin{bmatrix}
	1 & 2 & 3 & 4 & 5 & 6 & 7\\
	4 & 2 & 5 & 1 & 7 & 6 & 3
	\end{bmatrix}
$$

You can verify that this indeed does give you the inverse.

The inverse of an inverse is itself, i.e. \((\sigma^{-1})^{-1} = \sigma\), and this is true for any function (that is invertible).

Also, the inverse of any 2-cycle is itself, so \( \begin{pmatrix}a & b\end{pmatrix}^{-1} = \begin{pmatrix}a & b\end{pmatrix} \) for any numbers \(a\) and \(b\).


## Extended Compositions
One thing we can do is compose a permutation with itself, i.e. \(\sigma \circ \sigma\), which leads to some interesting results. As a shorthand we write: \(\sigma \circ \sigma = \sigma^2\), understanding that the superscript here refers to composition and NOT exponentiation.

We can generalize this to any integer \(n > 0\) so that:

$$ \sigma^n = \underbrace{\sigma \circ \sigma \circ \cdots \circ \sigma}_\text{$n$ times} $$

we can extend to the number 0 by defining:

$$ \sigma^0 = e $$

and, we can also extend into the negative integers by defining the following:

$$
	\sigma^{-n} = (\sigma^{-1})^n = \underbrace{\sigma^{-1} \circ \sigma^{-1} \circ \cdots \circ \sigma^{-1}}_\text{$n$ times}
$$

where \(n > 0\).

With this cool notation, we have found a way to use any integers \(k\) and \(r\), negative or non-negative, so that the following expressions are well-defined and true:

$$ \sigma^k \circ \sigma^r = \sigma^{k + r}, (\sigma^k)^r = \sigma^{kr} $$

This gives us the ability to "treat" compositions similar to exponent rules as seen in normal arthmetic. Of course, our superscript notation is NOT exponentiation, but it *behaves* similar to exponent rules.

An interesting question might be to ask for what values of \(k\) does \(\sigma^k = e\), if any at all. The answer is that this is true for any *finite* permutation.

If you think about, there cannot be an infinite amount of different permutations generated by a permutation being composed with itself over and over again. Eventually, it will have to start repeating itself, as there are only \(n!\) possible permutations. This means that there exists an integer \(k\) such that \(\sigma^k = \sigma\). And in fact, if this is true then we can prove that:

$$
	\begin{align}
	\sigma^k &= \sigma\\
	\Longrightarrow \sigma^{-1} \circ \sigma^k &= \sigma^{-1} \circ \sigma\\
	\Longrightarrow \sigma^{k-1} &= e
	\end{align}
$$

So we can state as a theorem that for every finite permutation, there exists an integer \(r\) such that \(\sigma^r = e\), \(\sigma^{r-1} = \sigma^{-1}\), and \(\sigma^{r+1} = \sigma \). This means that every permutation, when repeatedly applied over and over again, will eventually just lead all numbers back to themselves.

Let us define the set of all self compositions of \(\sigma\) as: \(\langle \sigma \rangle\), which is defined as:

$$ \langle \sigma \rangle\ = \{\sigma^n | n > 0\} $$

We restrict \(n\) to be greater than 0, but this is not neccessary. We could say it's less than or equal to 0, or we could just let it be any integer. All these choices will create the same set, as the set of self-compositions is cyclic. You could go backwards or forwards but it doesn't matter, you'll get the same results whichever direction you pick and whatever element you start with.

In fact, because of the cyclic nature of this set, we only need to specify an integer **range** that needs to be at least greater or equal to the number of elements in \(\langle \sigma \rangle\).

We define the **order** of \(\sigma\), denoted  by \(|\sigma|\), to be the smallest positive integer \(k\) such that \(\sigma^k = e\). Therefore:

$$ |\sigma| = |\langle \sigma \rangle| $$

And so we can give an equivalent definition for \(\langle \sigma \rangle\) as so:

$$ \langle \sigma \rangle = \{\sigma^k | k \in [a,b) \land |b - a| = | \sigma | \} $$

This notation is confusing for many reasons so I'll break it down:

- \(a\) and \(b\) are any integers
- \(|\sigma|\) refers to the *order* of \(\sigma\), whereas \(|b - a|\) refers to the *absolute value* of \(b-a\).
- The differing parentheses in \([a,b)\) is done on purpose so that \(b\) is not included. Otherwise, if we used the range \([a,b]\) instead, then we would have to require that \(b - a + 1 = | \sigma| \).

Try to understand this "cyclic" notion well, because it is a critical aspect of not just permutations but any group in general.


## Array & ODC Form
So far we've mentioned two main ways to represent permutations: tabular form and cycle form. Both of these representations have the property that there is more than one possible way of representing the same exact permutation.

In tabular form, it is possible to reorder columns freely without changing the permutation. In cycle form, we observed that disjoint cycles compose in any order that we like. Not only that, but the numbers within cycles can be re-ordered to start at any of the elements.

In order to introduce form that has a single representation per permutation, we will introduce **array form** and **ordered disjoint cycle form**.

**Array form** is written as so:

$$
	\begin{bmatrix}
	a_1 & a_2 & a_3 & \cdots & a_n
	\end{bmatrix}
$$

Where \(n\) is the amount of numbers to be permuted, and every \(a_i\) is unique and belongs to the set \(\{1,2,...,n\}\), and the permutation of the array form \(\tau\) is given by:

$$ \forall i \in \{1,2,...,n\} : \tau(i) = a_i $$

This can be best understood as imagining that there is a row above the array that has the numbers 1,2,3,...,n in order so that the following is true:

$$
	\begin{bmatrix}
	a_1 & a_2 & a_3 & \cdots & a_n
	\end{bmatrix}
	=
	\begin{bmatrix}
	1 & 2 & 3 & \cdots & n\\
	a_1 & a_2 & a_3 & \cdots & a_n
	\end{bmatrix}
$$

This basically just gives us a shorthand as opposed to tabular form. Array form is especially useful for computation as you can simply store vectors all of size \(n\) and easily compute compositions with very simple algorithms. The downside is that array form (like tabular form) is very "wasteful" when we have a lot of 1-cycles. Consider this example:

$$
	\begin{pmatrix}
	4 & 9
	\end{pmatrix}
	=
	\begin{bmatrix}
	1 & 2 & 3 & 9 & 5 & 6 & 7 & 8 & 4
	\end{bmatrix}
$$

It's not hard to see how array form could be a huge waste of memory over the much more memory-conservative cycle form. There are ways you could further optimize array form by using some kind of compression like [run-length encoding](https://en.wikipedia.org/wiki/Run-length_encoding) for runs of 1-cycles, but we won't go over that in this post.

We can also use **ordered disjoint cycle form** or just "ODC" for short, which is simply just using disjoint cycles where each cycle starts with the least number in that cycle. Since compositions of disjoint cycles can be rearranged, we also order the cycles by least starting element to greatest starting element.

Take this as an example conversion from normal cycle form to ODC form:

$$
	\begin{pmatrix}
	5 & 3
	\end{pmatrix}
	\begin{pmatrix}
	6 & 2 & 1
	\end{pmatrix}
	\begin{pmatrix}
	4 & 9
	\end{pmatrix}
	=
	\begin{pmatrix}
	1 & 6 & 2
	\end{pmatrix}
	\begin{pmatrix}
	3 & 5
	\end{pmatrix}
	\begin{pmatrix}
	4 & 9
	\end{pmatrix}
$$

ODC form also has the nice property that there is unique representation for every permutation, so we can compare two permutations in ODC form very easily to see if they are equal by simply iterating linearly over the numbers. The disadvantage with ODC compared to Array form is that with ODC, we need to store a list of lists to represent the cycles, which is more complex than just a single linear array. The tradeoff here is that ODC saves a lot more memory, and compositions with ODC can potentially be computed with significantly less iterations.


## Transpositions
The last thing I want to cover in this post are **transpositions**, which is simply just another name for a 2-cycle, i.e. \( \begin{pmatrix}a & b\end{pmatrix} \). We give 2-cycles this special name because of a very special property: *every permutation can be represented as a composition of transpositions*.

To prove this, we observe that the following is true for any k-cycle:

$$
	\begin{pmatrix}
	a_1 & a_2 & a_3 & \cdots & a_{k-1} & a_{k}
	\end{pmatrix}
	=
	\begin{pmatrix}
	a_1 & a_k
	\end{pmatrix}
	\begin{pmatrix}
	a_1 & a_{k-1}
	\end{pmatrix}
	\cdots
	\begin{pmatrix}
	a_1 & a_3
	\end{pmatrix}
	\begin{pmatrix}
	a_1 & a_2
	\end{pmatrix}
$$

The composition of these transpositions certainly aren't *disjoint* (as \(a_1\) appears in all of them), but they will equal the original cycle.

The next step in the proof is to realize that every permutation can be represented as a combination of cycles, which we have already implicity assumed. Then, if you take each cycle, simply replace it with it's equivalent composition of transpositions. The whole expression will then be one giant composition of transpositions, and it will be equal to the original permutation!

This result is useful because it means we can always represent a permutation as a series of "swaps". In other words, every permutation is just taking a list of things and swapping them around some number of times.
