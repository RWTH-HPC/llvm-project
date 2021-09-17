/*
 * critPathAnalysis.cpp -- Critical path analysis runtime library, build for
 * hybrid OpenMp and MPI applications
 */

#include "criticalPath.h"
#include <mpi.h>
#include <execinfo.h>

#ifdef FAKEMPI
#define MPI_CALL_PREFIX(f) MPI_##f
#define MPI_FUNC_PREFIX(f) XMPI_##f
#else
#define MPI_CALL_PREFIX(f) PMPI_##f
#define MPI_FUNC_PREFIX(f) MPI_##f
#endif

#define Call_Init MPI_CALL_PREFIX(Init)
#define Func_Init MPI_FUNC_PREFIX(Init)
#define Call_Init_thread MPI_CALL_PREFIX(Init_thread)
#define Func_Init_thread MPI_FUNC_PREFIX(Init_thread)
#define Call_Isend MPI_CALL_PREFIX(Isend)
#define Func_Isend MPI_FUNC_PREFIX(Isend)
#define Call_Irecv MPI_CALL_PREFIX(Irecv)
#define Func_Irecv MPI_FUNC_PREFIX(Irecv)
#define Call_Recv MPI_CALL_PREFIX(Recv)
#define Func_Recv MPI_FUNC_PREFIX(Recv)
#define Call_Send MPI_CALL_PREFIX(Send)
#define Func_Send MPI_FUNC_PREFIX(Send)
#define Call_Barrier MPI_CALL_PREFIX(Barrier)
#define Func_Barrier MPI_FUNC_PREFIX(Barrier)
#define Call_Finalize MPI_CALL_PREFIX(Finalize)
#define Func_Finalize MPI_FUNC_PREFIX(Finalize)
#define Call_Wait MPI_CALL_PREFIX(Wait)
#define Func_Wait MPI_FUNC_PREFIX(Wait)
#define Call_Waitall MPI_CALL_PREFIX(Waitall)
#define Func_Waitall MPI_FUNC_PREFIX(Waitall)
#define Call_Test MPI_CALL_PREFIX(Test)
#define Func_Test MPI_FUNC_PREFIX(Test)
#define Call_Testall MPI_CALL_PREFIX(Testall)
#define Func_Testall MPI_FUNC_PREFIX(Testall)

#define Call_Issend MPI_CALL_PREFIX(Issend)
#define Func_Issend MPI_FUNC_PREFIX(Issend)
#define Call_Reduce MPI_CALL_PREFIX(Reduce)
#define Func_Reduce MPI_FUNC_PREFIX(Reduce)
#define Call_Allreduce MPI_CALL_PREFIX(Allreduce)
#define Func_Allreduce MPI_FUNC_PREFIX(Allreduce)
#define Call_Bcast MPI_CALL_PREFIX(Bcast)
#define Func_Bcast MPI_FUNC_PREFIX(Bcast)
#define Call_Alltoall MPI_CALL_PREFIX(Alltoall)
#define Func_Alltoall MPI_FUNC_PREFIX(Alltoall)
#define Call_Alltoallv MPI_CALL_PREFIX(Alltoallv)
#define Func_Alltoallv MPI_FUNC_PREFIX(Alltoallv)
#define Call_Alltoallw MPI_CALL_PREFIX(Alltoallw)
#define Func_Alltoallw MPI_FUNC_PREFIX(Alltoallw)
#define Call_Ialltoallw MPI_CALL_PREFIX(Ialltoallw)
#define Func_Ialltoallw MPI_FUNC_PREFIX(Ialltoallw)
#define Call_Iallreduce MPI_CALL_PREFIX(Iallreduce)
#define Func_Iallreduce MPI_FUNC_PREFIX(Iallreduce)

#define Call_Cart_create MPI_CALL_PREFIX(Cart_create)
#define Func_Cart_create MPI_FUNC_PREFIX(Cart_create)
#define Call_Cart_sub MPI_CALL_PREFIX(Cart_sub)
#define Func_Cart_sub MPI_FUNC_PREFIX(Cart_sub)
#define Call_Comm_free MPI_CALL_PREFIX(Comm_free)
#define Func_Comm_free MPI_FUNC_PREFIX(Comm_free)

#define Call_Comm_create MPI_CALL_PREFIX(Comm_create)
#define Func_Comm_create MPI_FUNC_PREFIX(Comm_create)
#define Call_Comm_split MPI_CALL_PREFIX(Comm_split)
#define Func_Comm_split MPI_FUNC_PREFIX(Comm_split)
#define Call_Allgatherv MPI_CALL_PREFIX(Allgatherv)
#define Func_Allgatherv MPI_FUNC_PREFIX(Allgatherv)

#define NUM_UC_TIMERS 4

std::mutex req_mutex;

typedef enum { ISEND, IRECV, ICOLL } KIND;
struct uc_struct {
  uc_struct(double* _uc, KIND _kind, MPI_Request req, int remote = -1) : uc(_uc), kind(_kind), pb_req(req), remote(remote) {assert(remote>=-1);}
  uc_struct(const uc_struct& o) : uc(o.uc), kind(o.kind), pb_req(o.pb_req), next(o.next), remote(o.remote) {}
  uc_struct(): uc(NULL), kind(ISEND), pb_req(MPI_REQUEST_NULL) {}
  double* uc;
  KIND kind;
  MPI_Request pb_req;
  uc_struct* next{nullptr};
  int remote{-42};
};

std::map<MPI_Request, uc_struct> uc_map;
uc_struct uc_none{};

void insertUC(double* uc, MPI_Request req, KIND kind, MPI_Request pb_req, int remote=-1){
    const std::lock_guard<std::mutex> lock(req_mutex);
    auto iteratorPair = uc_map.insert(
        {req, uc_struct(uc, kind, pb_req, remote)});
    if (iteratorPair.second == false) {
/*      int flag;
      PMPI_Test(&iteratorPair.first->second.pb_req, &flag, MPI_STATUS_IGNORE);
      printf("insertUC(%x) already in map, tested: %i\n", req, flag);*/
/*      if (iteratorPair.first->second.uc)
        delete[] iteratorPair.first->second.uc;
      iteratorPair.first->second.uc = uc;
      iteratorPair.first->second.kind = kind;
      iteratorPair.first->second.pb_req = pb_req;
      iteratorPair.first->second.remote = remote;*/
      uc_struct* tmp = &iteratorPair.first->second;
      while(tmp->next)
        tmp = tmp->next;
      tmp->next = new uc_struct(uc, kind, pb_req, remote);
    }
}

uc_struct popUC(MPI_Request req){
    const std::lock_guard<std::mutex> lock(req_mutex);
    auto iteratorPair = uc_map.find(req);
//    assert(iteratorPair != uc_map.end());
    if (iteratorPair == uc_map.end())
      return uc_none;
    uc_struct uc = iteratorPair->second;
    if (!iteratorPair->second.next){
      uc_map.erase(iteratorPair);
    } else {
      uc_struct* tmp = iteratorPair->second.next;
      iteratorPair->second = *tmp;
      delete tmp;
    }
    return uc;
}

MPI_Comm cw_dup;
MPI_Comm cs_dup;
std::map<MPI_Comm, MPI_Comm> dupCommMap;
std::mutex dupCommMutex;
std::vector<double> timeOffsets;

MPI_Comm getDupComm(MPI_Comm comm, const char* func){
  if(comm == MPI_COMM_WORLD)
    return cw_dup;
  if(comm == MPI_COMM_SELF)
    return cs_dup;
  const std::lock_guard<std::mutex> lock(dupCommMutex);
  auto dupComm = dupCommMap.find(comm);
  if (dupComm == dupCommMap.end()) {
    int size, rank;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    printf("Could not find comm %x used in %s (%i, %i)\n", comm, func, rank, size);
              int j, nptrs;
       #define SIZE 100
           void *buffer[100];
           char **strings;

           nptrs = backtrace(buffer, SIZE);
           printf("backtrace() returned %d addresses\n", nptrs);

           /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
              would produce similar output to the following: */

           strings = backtrace_symbols(buffer, nptrs);
           if (strings == NULL) {
               perror("backtrace_symbols");
               exit(EXIT_FAILURE);
           }

           for (j = 0; j < nptrs; j++)
               printf("%s\n", strings[j]);

           free(strings);

    return MPI_COMM_NULL;
  }
  return dupComm->second;
}
#define getDupComm(comm) getDupComm(comm, __func__)

void makeCommDup(MPI_Comm comm, const char* func){
//  printf("makeCommDup(%x, %s)\n", comm, func);
  if(comm == MPI_COMM_WORLD){
    PMPI_Comm_dup(comm, &cw_dup);
    return;
  }
  if(comm == MPI_COMM_SELF){
    PMPI_Comm_dup(comm, &cs_dup);
    return;
  }
  MPI_Comm dupComm;
  PMPI_Comm_dup(comm, &dupComm);
  const std::lock_guard<std::mutex> lock(dupCommMutex);
  dupCommMap[comm] = dupComm;
}
#define makeCommDup(comm) makeCommDup(comm, __func__)

void freeCommDup(MPI_Comm comm){
  if(comm == MPI_COMM_WORLD) {
    PMPI_Comm_free(&cw_dup);
    return;
  }
  if(comm == MPI_COMM_SELF) {
    PMPI_Comm_free(&cs_dup);
    return;
  }
  const std::lock_guard<std::mutex> lock(dupCommMutex);
  auto dupComm = dupCommMap.find(comm);
  if (dupComm == dupCommMap.end())
    return;
  PMPI_Comm_free(&(dupComm->second));
  dupCommMap.erase(dupComm);
}



struct mpiTimer {
  const char* loc;
  mpiTimer(bool openmp_thread = false, const char* loc=NULL): loc(loc) {
    if (thread_local_clock == nullptr) {
      thread_local_clock =
          new THREAD_CLOCK(my_next_id(), 0, openmp_thread);
      if (thread_local_clock->stopped_mpi_clock == false)
        resetMpiClock(thread_local_clock);
      if (thread_local_clock->stopped_omp_clock == true)
        thread_local_clock->Start(OMP_ONLY, __func__);
    }else{
      thread_local_clock->Stop(MPI, loc);
    }
    assert(thread_local_clock->stopped_mpi_clock == true);
  }
  ~mpiTimer() { thread_local_clock->Start(MPI, loc); }
};

void MpiHappensAfter(double* critTimeFromRequest, int remote=0) {
  if (!analysis_flags->running)
    return;
  assert(thread_local_clock->stopped_mpi_clock == true);
  //thread_local_clock->Print("MpiHappensAfter-begin");
  assert(remote >= -1);
  double offset = 0;
  if (remote!=-1)
    offset = timeOffsets[remote];
  update_maximum(thread_local_clock->useful_computation_critical,
                 critTimeFromRequest[0]);
//  printf("update_maximum(%lf, %lf)\n", thread_local_clock->outsidempi_critical.load(), critTimeFromRequest[1]);
  update_maximum(thread_local_clock->outsidempi_critical,
                 critTimeFromRequest[1]);
  update_maximum(thread_local_clock->outsideomp_critical,
                 critTimeFromRequest[2]-offset);
  update_maximum(thread_local_clock->outsideomp_critical_nooffset,
                 critTimeFromRequest[3]);
//  thread_local_clock->Print("MpiHappensAfter-end");
}

#if 0
void MpiHappensAfter(MPI_Request request, int remote=-1) {
    if (!analysis_flags->running)
      return;
    const std::lock_guard<std::mutex> lock(req_mutex);
    auto it = uc_map.find(request);
    if (it != uc_map.end()) {
      if(it->second.kind == IRECV){
      double *critTimeFromRequest = it->second.uc;
/*      update_maximum(thread_local_clock->useful_computation_critical,
                     critTimeFromRequest[0]);
      update_maximum(thread_local_clock->outsidempi_critical,
                     critTimeFromRequest[1]);*/
        MpiHappensAfter(critTimeFromRequest, remote);
      }
      if (it->second.uc)
        delete[] it->second.uc;
      uc_map.erase(it);
  }
  auto& uc = popUC(request);
  if (uc != uc_none){
    
  }
}
#endif
/*
 * MPI
 */

void init_processes(mpiTimer& mt) { 
  PMPI_Comm_rank(MPI_COMM_WORLD, &myProcId); 
  int size;
  PMPI_Comm_size(MPI_COMM_WORLD, &size); 
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  printf("Process %i at %s\n", myProcId, processor_name);
  PMPI_Barrier(MPI_COMM_WORLD);
  double localTime = getTime();
  timeOffsets.resize(size);
  PMPI_Allgather(&localTime, 1, MPI_DOUBLE, 
                 timeOffsets.data(), 1, MPI_DOUBLE, MPI_COMM_WORLD);
  if(myProcId==0)
    printf("Timer Offsets:");
  for(auto& v : timeOffsets){
    v-=localTime;
    if(myProcId==0)
      printf("%lf, ", v);
  }
  if(myProcId==0)
  printf("\n");
  makeCommDup(MPI_COMM_WORLD);
  makeCommDup(MPI_COMM_SELF);
  if (!analysis_flags->running) {
    startTool(false,ALL);
    resetMpiClock(thread_local_clock);
  }
}

#ifdef FAKEMPI
extern "C" {
#endif

int Func_Init(int *argc, char ***argv) {
  if (!analysis_flags) {
    const char *options = getenv("ANALYSIS_OPTIONS");
    analysis_flags = new AnalysisFlags(options);
  }
  std::cout << "Starting critPathAnalysis tool" << std::endl;
  useMpi = true;

  mpiTimer mt{true, __func__};

  std::cout << "MPI Init" << std::endl;

  int ret = Call_Init(argc, argv);

  init_processes(mt);

//  thread_local_clock->Print("MPI_Init2");
  return ret;
}
int Func_Init_thread(int *argc, char ***argv, int required, int *provided) {
  if (!analysis_flags) {
    const char *options = getenv("ANALYSIS_OPTIONS");
    analysis_flags = new AnalysisFlags(options);
  }
  useMpi = true;
  mpiTimer mt{true, "mpiTimer in Func_Init_thread"};

  std::cout << "MPI Init Thread" << std::endl;

  int ret = Call_Init_thread(argc, argv, required, provided);
  init_processes(mt);
//  thread_local_clock->Print("MPI_Init_thread2");
  return ret;
}

int Func_Cart_create(MPI_Comm comm_old, int ndims, const int *dims,
                           const int *periods, int reorder, MPI_Comm *comm_cart){
  int ret = Call_Cart_create(comm_old, ndims, dims, periods, reorder, comm_cart);
  makeCommDup(*comm_cart);
  return ret;
}

int Func_Cart_sub(MPI_Comm comm, const int *remain_dims,
                        MPI_Comm *comm_new){
  int ret = Call_Cart_sub(comm, remain_dims, comm_new);
  makeCommDup(*comm_new);
  return ret;
}

int Func_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm){
  int ret = Call_Comm_create(comm, group, newcomm);
  makeCommDup(*newcomm);
  return ret;
}

int Func_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm){
  int ret = Call_Comm_split(comm, color, key, newcomm);
  makeCommDup(*newcomm);
  return ret;
}



int Func_Comm_free(MPI_Comm* comm){
  freeCommDup(*comm);
  return Call_Comm_free(comm);
}


int Func_Isend(const void *buf, int count, MPI_Datatype datatype, int dest,
              int tag, MPI_Comm comm, MPI_Request *request) {
  mpiTimer mt{false, __func__};
  thread_local_clock->isend++;
  // MpiHappensBefore(thread_local_clock->useful_computation_thread.load());
  // std::cout << thread_local_clock->useful_computation_thread.load() <<
  // std::endl; std::cout <<
  // thread_local_clock->useful_computation_critical.load() << std::endl;
  // piggybacking the useful computation as payload with the buffer

  double* uc = new double[NUM_UC_TIMERS]{thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load(),
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  
/*  MPI_Get_address(uc, &payloadAddress);
  MPI_Datatype newtype;
  const int array_of_block_lengths[] = {3, count};
  const MPI_Aint array_of_displacements[] = {payloadAddress, (MPI_Aint)buf};
  const MPI_Datatype array_of_types[] = {MPI_DOUBLE, datatype};
  MPI_Type_create_struct(2, array_of_block_lengths, array_of_displacements,
                          array_of_types, &newtype);
  MPI_Type_commit(&newtype);*/
  MPI_Request pb_req=MPI_REQUEST_NULL;
  PMPI_Isend(uc, NUM_UC_TIMERS, MPI_DOUBLE, dest, tag, getDupComm(comm), &pb_req);

  int send_ret = Call_Isend(buf, count, datatype, dest, tag, comm, request);
  //MPI_Type_free(&newtype);
  int flag;
  PMPI_Test(&pb_req, &flag, MPI_STATUS_IGNORE);
  if(!flag)
    insertUC(uc, *request, ISEND, pb_req);

  return send_ret;
}
int Func_Issend(const void *buf, int count, MPI_Datatype datatype, int dest,
              int tag, MPI_Comm comm, MPI_Request *request) {
  mpiTimer mt{false, __func__};
  thread_local_clock->isend++;
  // MpiHappensBefore(thread_local_clock->useful_computation_thread.load());
  // std::cout << thread_local_clock->useful_computation_thread.load() <<
  // std::endl; std::cout <<
  // thread_local_clock->useful_computation_critical.load() << std::endl;
  // piggybacking the useful computation as payload with the buffer

  double* uc = new double[NUM_UC_TIMERS]{thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load(),
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  
/*  MPI_Get_address(uc, &payloadAddress);
  MPI_Datatype newtype;
  const int array_of_block_lengths[] = {3, count};
  const MPI_Aint array_of_displacements[] = {payloadAddress, (MPI_Aint)buf};
  const MPI_Datatype array_of_types[] = {MPI_DOUBLE, datatype};
  MPI_Type_create_struct(2, array_of_block_lengths, array_of_displacements,
                          array_of_types, &newtype);
  MPI_Type_commit(&newtype);*/
  MPI_Request pb_req=MPI_REQUEST_NULL;
  //TODO: add backwards synchronization
  PMPI_Isend(uc, NUM_UC_TIMERS, MPI_DOUBLE, dest, tag, getDupComm(comm), &pb_req);

  int send_ret = Call_Issend(buf, count, datatype, dest, tag, comm, request);
  //MPI_Type_free(&newtype);
  int flag;
  PMPI_Test(&pb_req, &flag, MPI_STATUS_IGNORE);
  if(!flag)
    insertUC(uc, *request, ISEND, pb_req);

  return send_ret;
}

int Func_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
              MPI_Comm comm, MPI_Request *request) {
  mpiTimer mt{false, __func__};
  thread_local_clock->irecv++;

  double* uc = new double[NUM_UC_TIMERS];

  // actual mpi receive wrapped with PMPI
  MPI_Request pb_req=MPI_REQUEST_NULL;
  PMPI_Irecv(uc, NUM_UC_TIMERS, MPI_DOUBLE, source, tag, getDupComm(comm), &pb_req);
  int recv_ret = Call_Irecv(buf, count, datatype, source, tag, comm, request);

  insertUC(uc, *request, IRECV, pb_req, source);

  return recv_ret;
}

int Func_Ialltoallw(const void *sendbuf, const int *sendcnts,
                         const int *sdispls,
                         const MPI_Datatype *sendtypes, void *recvbuf,
                         const int *recvcnts, const int *rdispls,
                         const MPI_Datatype *recvtypes, MPI_Comm comm, MPI_Request *request) {
  // end useful computation
  mpiTimer mt{false, __func__};
  thread_local_clock->icoll++;
  double* uc = new double[NUM_UC_TIMERS]{thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  MPI_Request pb_req=MPI_REQUEST_NULL;
  if (analysis_flags->running) 
    PMPI_Iallreduce(MPI_IN_PLACE, uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(comm), &pb_req);

  int ret =
      Call_Ialltoallw(sendbuf, sendcnts, sdispls, sendtypes,
                     recvbuf, recvcnts, rdispls, recvtypes, comm, request); // execute the MPI_Barrier from the MPI lib
  if (analysis_flags->running) 
    insertUC(uc, *request, ICOLL, pb_req);
  return ret;
}


// MPI recv call intercepted from library
int Func_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
             MPI_Comm comm, MPI_Status *status) {
  mpiTimer mt{false, __func__};
  thread_local_clock->recv++;

  // piggybacking the useful computation as payload with the buffer
  //MPI_Datatype newtype;
  double uc_recv[NUM_UC_TIMERS] = {0,0,0};
/*  MPI_Aint payloadAddress;
  MPI_Get_address(uc_recv, &payloadAddress);
  const int array_of_block_lengths[] = {NUM_UC_TIMERS, count};
  const MPI_Aint array_of_displacements[] = {payloadAddress, (MPI_Aint)buf};
  const MPI_Datatype array_of_types[] = {MPI_DOUBLE, datatype};
  MPI_Type_create_struct(2, array_of_block_lengths, array_of_displacements,
                          array_of_types, &newtype);
  MPI_Type_commit(&newtype);*/

  // actual mpi receive wrapped with PMPI
  PMPI_Recv(uc_recv, NUM_UC_TIMERS, MPI_DOUBLE, source, tag, getDupComm(comm), MPI_STATUS_IGNORE);
  int recv_ret = Call_Recv(buf, count, datatype, source, tag, comm, status);
//  MPI_Type_free(&newtype);

  // happens relation to update clock
  MpiHappensAfter(uc_recv, source);

  return recv_ret;
}

int Func_Send(const void *buf, int count, MPI_Datatype datatype, int dest,
             int tag, MPI_Comm comm) {
  mpiTimer mt{false, __func__};
  thread_local_clock->send++;

  // piggybacking the useful computation as payload with the buffer
  double my_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load(),
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  /*MPI_Aint payloadAddress;
  MPI_Get_address(&my_uc, &payloadAddress);
  MPI_Datatype newtype;
  const int array_of_block_lengths[] = {NUM_UC_TIMERS, count};
  const MPI_Aint array_of_displacements[] = {payloadAddress, (MPI_Aint)buf};
  const MPI_Datatype array_of_types[] = {MPI_DOUBLE, datatype};
  MPI_Type_create_struct(2, array_of_block_lengths, array_of_displacements,
                          array_of_types, &newtype);
  MPI_Type_commit(&newtype);*/

  PMPI_Send(my_uc, NUM_UC_TIMERS, MPI_DOUBLE, dest, tag, getDupComm(comm));
  int send_ret = Call_Send(buf, count, datatype, dest, tag, comm);
  //MPI_Type_free(&newtype);

  return send_ret;
}

int Func_Barrier(MPI_Comm comm) {
  // end useful computation
  mpiTimer mt{false, __func__};
  thread_local_clock->coll++;
if (analysis_flags->running) {
  double max_uc[NUM_UC_TIMERS],
      local_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  PMPI_Allreduce(&local_uc, &max_uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(comm));
  thread_local_clock->useful_computation_critical = max_uc[0];
  thread_local_clock->outsidempi_critical = max_uc[1];
  thread_local_clock->outsideomp_critical = max_uc[2]+timeOffsets[0];
  thread_local_clock->outsideomp_critical_nooffset = max_uc[3];
}
  int barrier_ret =
      Call_Barrier(comm); // execute the MPI_Barrier from the MPI lib
  return barrier_ret;
}

int Func_Bcast( void *buffer, int count, MPI_Datatype datatype, int root,
                      MPI_Comm comm ) {
  // end useful computation
  mpiTimer mt{false, __func__};
  thread_local_clock->coll++;
if (analysis_flags->running) {
  double max_uc[NUM_UC_TIMERS],
      local_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  // TODO: fix to use MPI_Bcast
  PMPI_Allreduce(&local_uc, &max_uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(comm));
  thread_local_clock->useful_computation_critical = max_uc[0];
  thread_local_clock->outsidempi_critical = max_uc[1];
  thread_local_clock->outsideomp_critical = max_uc[2]+timeOffsets[0];
  thread_local_clock->outsideomp_critical_nooffset = max_uc[3];
}
  int barrier_ret =
      Call_Bcast(buffer, count, datatype, root, comm); // execute the MPI_Barrier from the MPI lib
  return barrier_ret;
}

int Func_Alltoallw(const void *sendbuf, const int *sendcnts,
                         const int *sdispls,
                         const MPI_Datatype *sendtypes, void *recvbuf,
                         const int *recvcnts, const int *rdispls,
                         const MPI_Datatype *recvtypes, MPI_Comm comm) {
  // end useful computation
  mpiTimer mt{false, __func__};
  thread_local_clock->coll++;
if (analysis_flags->running) {
  double max_uc[NUM_UC_TIMERS],
      local_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  // TODO: fix to use MPI_Bcast
  PMPI_Allreduce(&local_uc, &max_uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(comm));
  thread_local_clock->useful_computation_critical = max_uc[0];
  thread_local_clock->outsidempi_critical = max_uc[1];
  thread_local_clock->outsideomp_critical = max_uc[2]+timeOffsets[0];
  thread_local_clock->outsideomp_critical_nooffset = max_uc[3];
}
  int barrier_ret =
      Call_Alltoallw(sendbuf, sendcnts, sdispls, sendtypes,
                     recvbuf, recvcnts, rdispls, recvtypes, comm); // execute the MPI_Barrier from the MPI lib
  return barrier_ret;
}

int Func_Allgatherv(const void *sendbuf, int sendcount,
                          MPI_Datatype sendtype, void *recvbuf,
                          const int *recvcounts, const int *displs,
                          MPI_Datatype recvtype, MPI_Comm comm){
  // end useful computation
  mpiTimer mt{false, __func__};
  thread_local_clock->coll++;
if (analysis_flags->running) {
  double max_uc[NUM_UC_TIMERS],
      local_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  // TODO: fix to use MPI_Bcast
  PMPI_Allreduce(&local_uc, &max_uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(comm));
  thread_local_clock->useful_computation_critical = max_uc[0];
  thread_local_clock->outsidempi_critical = max_uc[1];
  thread_local_clock->outsideomp_critical = max_uc[2]+timeOffsets[0];
  thread_local_clock->outsideomp_critical_nooffset = max_uc[3];
}
  int barrier_ret =
      Call_Allgatherv(sendbuf, sendcount, sendtype,
                     recvbuf, recvcounts, displs, recvtype, comm); 
  return barrier_ret;
  
}


int Func_Alltoallv(const void *sendbuf, const int *sendcnts,
                         const int *sdispls,
                         MPI_Datatype sendtype, void *recvbuf,
                         const int *recvcnts, const int *rdispls,
                         MPI_Datatype recvtype, MPI_Comm comm) {
  // end useful computation
  mpiTimer mt{false, __func__};
  thread_local_clock->coll++;
if (analysis_flags->running) {
  double max_uc[NUM_UC_TIMERS],
      local_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  // TODO: fix to use MPI_Bcast
  PMPI_Allreduce(&local_uc, &max_uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(comm));
  thread_local_clock->useful_computation_critical = max_uc[0];
  thread_local_clock->outsidempi_critical = max_uc[1];
  thread_local_clock->outsideomp_critical = max_uc[2]+timeOffsets[0];
  thread_local_clock->outsideomp_critical_nooffset = max_uc[3];
}
  int barrier_ret =
      Call_Alltoallv(sendbuf, sendcnts, sdispls, sendtype,
                     recvbuf, recvcnts, rdispls, recvtype, comm); // execute the MPI_Barrier from the MPI lib
  return barrier_ret;
}

int Func_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype, 
                  void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
  // end useful computation
  mpiTimer mt{false, __func__};
  thread_local_clock->coll++;
if (analysis_flags->running) {
  double max_uc[NUM_UC_TIMERS],
      local_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  // TODO: fix to use MPI_Bcast
  PMPI_Allreduce(&local_uc, &max_uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(comm));
  thread_local_clock->useful_computation_critical = max_uc[0];
  thread_local_clock->outsidempi_critical = max_uc[1];
  thread_local_clock->outsideomp_critical = max_uc[2]+timeOffsets[0];
  thread_local_clock->outsideomp_critical_nooffset = max_uc[3];
}
  int barrier_ret =
      Call_Alltoall(sendbuf, sendcount, sendtype,
                    recvbuf, recvcount, recvtype, comm); // execute the MPI_Barrier from the MPI lib
  return barrier_ret;
}

int Func_Allreduce(const void *sendbuf, void *recvbuf, int count,
                         MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
  // end useful computation
  mpiTimer mt{false, __func__};
  thread_local_clock->coll++;
if (analysis_flags->running) {
  double max_uc[NUM_UC_TIMERS],
      local_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  // TODO: fix to use MPI_Bcast
  PMPI_Allreduce(&local_uc, &max_uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(comm));
  thread_local_clock->useful_computation_critical = max_uc[0];
  thread_local_clock->outsidempi_critical = max_uc[1];
  thread_local_clock->outsideomp_critical = max_uc[2]+timeOffsets[0];
  thread_local_clock->outsideomp_critical_nooffset = max_uc[3];
}
  int barrier_ret =
      Call_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
  return barrier_ret;
}

int Func_Reduce(const void *sendbuf, void *recvbuf, int count,
                         MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
  // end useful computation
  mpiTimer mt{false, __func__};
  thread_local_clock->coll++;
if (analysis_flags->running) {
  double max_uc[NUM_UC_TIMERS],
      local_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  // TODO: fix to use MPI_Bcast
  PMPI_Allreduce(&local_uc, &max_uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(comm));
  thread_local_clock->useful_computation_critical = max_uc[0];
  thread_local_clock->outsidempi_critical = max_uc[1];
  thread_local_clock->outsideomp_critical = max_uc[2]+timeOffsets[0];
  thread_local_clock->outsideomp_critical_nooffset = max_uc[3];
}
  int barrier_ret =
      Call_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
  return barrier_ret;
}

int Func_Finalize(void) {
  mpiTimer mt{false, __func__};
  //thread_local_clock->Print("MPI_Finalize1");
  if (ompt_finalize_tool)
    ompt_finalize_tool();
  //thread_local_clock->Print("MPI_Finalize2");
  assert(thread_local_clock->stopped_clock == true);
  assert(thread_local_clock->stopped_mpi_clock == true);
  double max_uc[NUM_UC_TIMERS],
      local_uc[NUM_UC_TIMERS] = {thread_local_clock->useful_computation_critical.load(),
                     thread_local_clock->outsidempi_critical.load(),
                     thread_local_clock->outsideomp_critical.load()-timeOffsets[0],
                     thread_local_clock->outsideomp_critical_nooffset.load()};
  PMPI_Allreduce(&local_uc, &max_uc, NUM_UC_TIMERS, MPI_DOUBLE, MPI_MAX, getDupComm(MPI_COMM_WORLD));
  thread_local_clock->useful_computation_critical = max_uc[0];
  thread_local_clock->outsidempi_critical = max_uc[1];
  thread_local_clock->outsideomp_critical = max_uc[2]+timeOffsets[0];
  thread_local_clock->outsideomp_critical_nooffset = max_uc[3];
  finishMeasurement();
  freeCommDup(MPI_COMM_WORLD);
  freeCommDup(MPI_COMM_SELF);
  // execute the MPI finalize command
  return Call_Finalize();
}

int Func_Wait(MPI_Request *request, MPI_Status *status) {
  mpiTimer mt{false, __func__};
  //double *critTimeFromRequest;
  thread_local_clock->wait++;
  uc_struct uc = popUC(*request);
/*  {
    const std::lock_guard<std::mutex> lock(req_mutex);
    auto it = uc_map.find(*request);
    if (it != uc_map.end()) {
      uc=it->second;
      uc_map.erase(it);
    }
  }*/


  //MPI_Request req = *request; // copy the request
  int ret = Call_Wait(request, status);
  PMPI_Wait(&uc.pb_req, MPI_STATUS_IGNORE);

  if(uc.kind != ISEND)
    MpiHappensAfter(uc.uc, uc.remote);
  if(uc.uc)
    delete[] uc.uc;

  return ret;
}
int Func_Waitall(int count, MPI_Request array_of_requests[],
                MPI_Status array_of_statuses[]) {
  mpiTimer mt{false, __func__};

  thread_local_clock->wait++;
  MPI_Request array_of_requests_copied[count];
  for (int i = 0; i < count; i++) {
    array_of_requests_copied[i] = array_of_requests[i];
  }

  int ret = Call_Waitall(count, array_of_requests, array_of_statuses);

  for (int i = 0; i < count; i++) {
    uc_struct uc = popUC(array_of_requests_copied[i]);
/*    {
      const std::lock_guard<std::mutex> lock(req_mutex);
      auto it = uc_map.find(array_of_requests_copied[i]);
      if (it != uc_map.end()) {
        uc=it->second;
        uc_map.erase(it);
      }
    }*/
    PMPI_Wait(&uc.pb_req, MPI_STATUS_IGNORE);
    if(uc.kind != ISEND)
      MpiHappensAfter(uc.uc, uc.remote);
    if(uc.uc)
      delete[] uc.uc;
  }

  return ret;
}

// tests for completion of request
int Func_Test(MPI_Request *request, int *flag, MPI_Status *status) {
  mpiTimer mt{false, __func__};
  MPI_Request req = *request; // copy the request

  thread_local_clock->test++;
  int test_ret = Call_Test(request, flag, status);

  if (*flag == true) {
    uc_struct uc = popUC(req);
/*    {
      const std::lock_guard<std::mutex> lock(req_mutex);
      auto it = uc_map.find(req);
      if (it != uc_map.end()) {
        uc=it->second;
        uc_map.erase(it);
      }
    }*/
    PMPI_Wait(&uc.pb_req, MPI_STATUS_IGNORE);
    if(uc.kind != ISEND){
      MpiHappensAfter(uc.uc, uc.remote);
    }
    if(uc.uc)
      delete[] uc.uc;
  }
  return test_ret;
}

// tests for the completion of all previously initiated requests
int Func_Testall(int count, MPI_Request array_of_requests[], int *flag,
                MPI_Status array_of_statuses[]) {
  mpiTimer mt{false, __func__};

  thread_local_clock->test++;
  MPI_Request array_of_requests_copied[count];
  for (int i = 0; i < count; i++) {
    array_of_requests_copied[i] = array_of_requests[i];
  }

  int testall_ret =
      Call_Testall(count, array_of_requests, flag, array_of_statuses);
  if (*flag == true) {
    for (int i = 0; i < count; i++) {
      uc_struct uc = popUC(array_of_requests_copied[i]);
/*      {
        const std::lock_guard<std::mutex> lock(req_mutex);
        auto it = uc_map.find(array_of_requests_copied[i]);
        if (it != uc_map.end()) {
          uc=it->second;
          uc_map.erase(it);
        }
      }*/
      PMPI_Wait(&uc.pb_req, MPI_STATUS_IGNORE);
      if(uc.kind != ISEND)
        MpiHappensAfter(uc.uc, uc.remote);
      if(uc.uc)
        delete[] uc.uc;
    }
  }

  return testall_ret;
}

int MPI_Pcontrol(const int level, ...){
  if (level == 1) {
    startTool();
  } else if (level == 0) {
    stopTool();
  }
  return MPI_SUCCESS;
}

#ifdef FAKEMPI
} // extern "C"
#endif

//#include "fortran-wrapper.h"
