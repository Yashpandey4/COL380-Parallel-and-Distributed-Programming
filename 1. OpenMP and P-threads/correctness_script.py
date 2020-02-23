# import sparse 
import scipy.sparse as sparse
# import stats
import scipy.stats as stats
# import numpy
import numpy as np
import scipy
import sys

# n = int(sys.argv[1])
A_file = sys.argv[1]
P_file = sys.argv[2]
L_file = sys.argv[3]
U_file = sys.argv[4]


def read_from_file(filename):
	# mat = mat.toarray()
	mat=[]
	with open(filename,'r') as f:
		for line in f:
			line1=line.rstrip().split(" ")
			mat.append(list(map(float, line1)))
			# f.write("\n")
	return np.asarray(mat)

def is_permuation_matrix(x):
    x = np.asanyarray(x)
    return (x.ndim == 2 and x.shape[0] == x.shape[1] and
            (x.sum(axis=0) == 1).all() and 
            (x.sum(axis=1) == 1).all() and
            ((x == 1) | (x == 0)).all())

A=read_from_file(A_file)
PA=read_from_file(P_file)
LA=read_from_file(L_file)
UA=read_from_file(U_file)
n = A.shape[0]
print('Size of the given matrix = {}\n'.format(n))
Q=np.matmul(LA,UA) 
Q1=np.matmul(PA,A)
Q2=np.abs(Q-Q1)

print('|PA - LU| = {}\n'.format(np.linalg.norm(Q2,2)))
print('Is L Lower Triangular? {}'.format(np.allclose(LA, np.tril(LA))))
print('Are the diagonal entries of L equal to 1? {}\n'.format(np.all(np.diag(LA)==np.ones(n))))
print('Is R Upper Triangular? {}\n'.format(np.allclose(UA, np.triu(UA))))
print('Is P a Valid Permutation Matrix? {}\n'.format(is_permuation_matrix(PA)))