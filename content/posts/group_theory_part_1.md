+++
date = '2025-03-09'
draft = false
title = 'Introduction to Group Theory Part 1: Permutations'
+++

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

This format is known as [Cauchy's two-line notation](https://en.wikipedia.org/wiki/Augustin-Louis_Cauchy), but we will refer to it as tabular form. All entries are read "top-down" per column. In any given column, the element of the top row is "sent" to the element of the bottom row.
