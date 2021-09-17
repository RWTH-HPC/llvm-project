



#if defined(MPICH_NAME) && (MPICH_NAME == 1) /* MPICH has no MPI_IN_PLACE */
#define BufferC2F(x) (IsBottom(x) ? MPI_BOTTOM : (x))
#else
#define BufferC2F(x) (IsBottom(x) ? MPI_BOTTOM : (IsInPlace(x) ? MPI_IN_PLACE : (x)))
#endif /* defined(MPICH_NAME) && (MPICH_NAME == 1) */

#else
#define BufferC2F(x) (x)
#endif /* defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__PGI) || defined(_CRAYC) */



/* =============== Fortran Wrappers for MPI_Alltoall =============== */
static void MPI_Alltoall_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    WRAP_MPI_CALL_PREFIX
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Alltoall(BufferC2F((const void*)sendbuf), *sendcount, (MPI_Datatype)(*sendtype), BufferC2F((void*)recvbuf), *recvcount, (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Alltoall(BufferC2F((const void*)sendbuf), *sendcount, MPI_Type_f2c(*sendtype), BufferC2F((void*)recvbuf), *recvcount, MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    WRAP_MPI_CALL_POSTFIX
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLTOALL(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoall(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoall_(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoall__(MPI_Fint *sendbuf, MPI_Fint *sendcount, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcount, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoall_fortran_wrapper(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Alltoall ================= */

/* =============== Fortran Wrappers for MPI_Alltoallv =============== */
static void MPI_Alltoallv_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint *sendcounts, MPI_Fint *sdispls, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcounts, MPI_Fint *rdispls, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    WRAP_MPI_CALL_PREFIX
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Alltoallv(BufferC2F((const void*)sendbuf), ((const int*)sendcounts), ((const int*)sdispls), (MPI_Datatype)(*sendtype), BufferC2F((void*)recvbuf), ((const int*)recvcounts), ((const int*)rdispls), (MPI_Datatype)(*recvtype), (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    _wrap_py_return_val = MPI_Alltoallv(BufferC2F((const void*)sendbuf), ((const int*)sendcounts), ((const int*)sdispls), MPI_Type_f2c(*sendtype), BufferC2F((void*)recvbuf), ((const int*)recvcounts), ((const int*)rdispls), MPI_Type_f2c(*recvtype), MPI_Comm_f2c(*comm));
#endif /* MPICH test */
    WRAP_MPI_CALL_POSTFIX
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLTOALLV(MPI_Fint *sendbuf, MPI_Fint *sendcounts, MPI_Fint *sdispls, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcounts, MPI_Fint *rdispls, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallv(MPI_Fint *sendbuf, MPI_Fint *sendcounts, MPI_Fint *sdispls, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcounts, MPI_Fint *rdispls, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallv_(MPI_Fint *sendbuf, MPI_Fint *sendcounts, MPI_Fint *sdispls, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcounts, MPI_Fint *rdispls, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallv__(MPI_Fint *sendbuf, MPI_Fint *sendcounts, MPI_Fint *sdispls, MPI_Fint *sendtype, MPI_Fint *recvbuf, MPI_Fint *recvcounts, MPI_Fint *rdispls, MPI_Fint *recvtype, MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallv_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm, ierr);
}

/* ================= End Wrappers for MPI_Alltoallv ================= */

/* =============== Fortran Wrappers for MPI_Alltoallw =============== */
static void MPI_Alltoallw_fortran_wrapper(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    int _wrap_py_return_val = 0;
    WRAP_MPI_CALL_PREFIX
#if (!defined(MPICH_HAS_C2F) && defined(MPICH_NAME) && (MPICH_NAME == 1)) /* MPICH test */
    _wrap_py_return_val = MPI_Alltoallw(BufferC2F((const void*)sendbuf), ((const int*)sendcounts), ((const int*)sdispls), (const MPI_Datatype*)sendtypes, BufferC2F((void*)recvbuf), ((const int*)recvcounts), ((const int*)rdispls), (const MPI_Datatype*)recvtypes, (MPI_Comm)(*comm));
#else /* MPI-2 safe call */
    MPI_Datatype temp_recvtypes;
    MPI_Datatype temp_sendtypes;
    _wrap_py_return_val = MPI_Alltoallw(BufferC2F((const void*)sendbuf), ((const int*)sendcounts), ((const int*)sdispls), &temp_sendtypes, BufferC2F((void*)recvbuf), ((const int*)recvcounts), ((const int*)rdispls), &temp_recvtypes, MPI_Comm_f2c(*comm));
    *sendtypes = MPI_Type_c2f(temp_sendtypes);
    *recvtypes = MPI_Type_c2f(temp_recvtypes);
#endif /* MPICH test */
    WRAP_MPI_CALL_POSTFIX
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_ALLTOALLW(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallw(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallw_(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

_EXTERN_C_ void mpi_alltoallw__(MPI_Fint *sendbuf, MPI_Fint sendcounts[], MPI_Fint sdispls[], MPI_Fint sendtypes[], MPI_Fint *recvbuf, MPI_Fint recvcounts[], MPI_Fint rdispls[], MPI_Fint recvtypes[], MPI_Fint *comm, MPI_Fint *ierr) { 
    MPI_Alltoallw_fortran_wrapper(sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm, ierr);
}

/* ================= End Wrappers for MPI_Alltoallw ================= */





