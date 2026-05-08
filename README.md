# Relative Neighbourhood Graph (RNG)
El grafo de vecinidad relativa (RNG) es un grafo geométrico no dirigido que conecta dos puntos $p$ y $q$ si no existe un tercer punto $r$ que sea más cercano a ambos que la distancia entre $p$ y $q$.
Es decir, la arista $(p, q)$ existe si y solo si:

$$\forall r \neq p, q: \text{max}(d(p, r), d(q, r)) > d(p, q)$$

<p align="center">
  <img src="rng.png" alt="RNG edge construction" width="300">
</p>

## Algoritmo
1. Para cada par de puntos $(p, q)$ en el conjunto:
2. Calcular la distancia $d(p, q)$
3. Verificar los demás puntos $r$:
   *Si algún $r$ cumple $d(p, r) \leq d(p, q)$ y $d(q, r) \leq d(p, q)$,se rechaza la arista
4. Si ningún $r$ cumple la condición, se agrega la arista $(p, q)$.


## Complejidad
$O(n^2)$ donde $n$ es el número de puntos.

## Propiedades
Es un subgrafo de la triangulación de Delaunay. Es un grafo conectado, invariante bajo rotaciones y traslaciones.
