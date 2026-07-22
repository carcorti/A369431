\\ Generate a(0),...,a(400) by expanding the PermPAL rational function.
\\ This intentionally does not iterate the recurrence used by the Python tool.
max_n = 400;
A = (x - 1) * (x^2 - 3*x + 1) / (4*x^3 - 7*x^2 + 5*x - 1) + O(x^(max_n + 1));
for (n = 0, max_n, print(n, " ", polcoef(A, n)));
print();
quit;
