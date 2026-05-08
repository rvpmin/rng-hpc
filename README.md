# Relative Neighbourhood Graph (RNG)
El grafo de vecinidad relativa (RNG) es un grafo geométrico no dirigido que conecta dos puntos $p$ y $q$ si no existe un tercer punto $r$ que sea más cercano a ambos que la distancia entre $p$ y $q$.
Es decir, la arista $(p, q)$ existe si y solo si:

$$\forall r \neq p, q: \text{max}(d(p, r), d(q, r)) > d(p, q)$$

