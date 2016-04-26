module helper_func
contains
  ! Adds array to the temporary buffer array
  subroutine save_temporary(in_array, index, tmp_index, tmp_storage)
    double precision, intent(in)::in_array(:)
    integer, intent(in)::index
    integer, intent(inout)::tmp_index(:)
    double precision, intent(inout)::tmp_storage(:,:)

    ! local variables
    integer :: n,i
    logical :: debug = .false.

    n = size(tmp_index)
    do i = 1,n
       if (tmp_index(i) == -1) then
          tmp_index(i) = index
          tmp_storage(1:size(in_array),i) = in_array
          debug = .true.
          exit
       end if
    end do

    if (.not. debug) then
       print *,"Failed to find tmp index"
    end if
  end subroutine save_temporary

  subroutine get_index(index, tmp_index, n)
    integer, intent(inout)::index
    integer, intent(in)::tmp_index(:)
    integer, intent(in) :: n

    ! local variables
    integer :: i
    logical :: debug = .false.

    do i=1,n
       if (index == tmp_index(i)) then
          index = tmp_index(i)
          debug = .true.
          exit
       end if
    end do
    if (.not. debug) then
       print *,"Failed to get tmp index"
    end if
  end subroutine get_index

  ! Clean the temporary buffer array
  subroutine clean_temporary(index, tmp_index, finished_list, n)
    integer, intent(in)::index
    logical, intent(inout)::finished_list(:)
    integer, intent(inout)::tmp_index(:)
    integer, intent(in) :: n

    if ( finished_list(index-1) .and. finished_list(index+1)) then
       tmp_index(index) = -1
    end if

    if ( index + 2 < n .and. finished_list(index + 1) .and. finished_list(index + 2)) then
       tmp_index(index + 1) = -1
    end if

    if ( index - 2 >= 0 .and. finished_list(index - 1) .and. finished_list(index - 2)) then
       tmp_index(index - 1) = -1
    end if

  end subroutine clean_temporary

end module helper_func

program laplsolv

  use omp_lib
  use helper_func
  implicit none

  integer, parameter                  :: n=100, maxiter=1000
  double precision,parameter          :: tol=1.0E-3
  double precision,dimension(0:n+1,0:n+1) :: T
  double precision                    :: error,x
  real                                :: t1,t0
  integer                             :: i,j,k
  logical,dimension(0:n+1)            :: finished_index
  integer,dimension(:),allocatable   :: temporary_index
  double precision,allocatable       :: temporary_storage(:,:)
  character(len=20)                   :: str
  integer                             :: tot_threads, t_id, storage_size

  ! Initialize all the finnished indicies to false (fortran boolean looks weird)
  finished_index(1:n) = .false.
  finished_index(0) = .true.
  finished_index(n+1) = .true.

  ! Set boundary conditions and initial values for the unknowns
  T=0.0D0
  T(0:n+1 , 0)     = 1.0D0
  T(0:n+1 , n+1)   = 1.0D0
  T(n+1   , 0:n+1) = 2.0D0

  !$omp parallel
  !$omp single
  tot_threads = omp_get_num_threads()
  !$omp end single
  !$omp end parallel

  ! Allocate and initialize the temporary data arrays
  storage_size = tot_threads*2
  allocate(temporary_index(storage_size))
  allocate(temporary_storage(n,storage_size))

  temporary_index(1:storage_size) = -1

  call cpu_time(t0)

  do k=1,maxiter

     !tmp1=T(1:n,0)
     call save_temporary(T(1:n,0),0,temporary_index,temporary_storage)
     error=0.0D0
     do j=1,4
        call save_temporary(T(1:n,j),j,temporary_index,temporary_storage)
        i = j-1
        call get_index(i,temporary_index,storage_size)
        !print *,i
        T(1:n,j)= &
             (T(0:n-1,j)+ &
             T(2:n+1,j)+ &
             T(1:n,j+1)+ &
             temporary_storage(1:n,i))/4.0D0
        i = j
        call get_index(i,temporary_index,storage_size)
        print *,i
        error=max( &
             error, maxval( &
             abs( temporary_storage(1:n,i)-T(1:n,j) &
             )))
        call clean_temporary(j,temporary_index,finished_index,n+2)
     end do

     if(error<tol) then
        exit
     end if
  end do

  call cpu_time(t1)

  write(unit=*,fmt=*) 'Time:',t1-t0,'Number of Iterations:',k
  write(unit=*,fmt=*) 'Temperature of element T(1,1)  =',T(1,1)

  ! Uncomment the next part if you want to write the whole solution
  ! to a file. Useful for plotting.

  !open(unit=7,action='write',file='result.dat',status='unknown')
  !write(unit=str,fmt='(a,i6,a)') '(',N,'F10.6)'
  !do i=0,n+1
  !   write (unit=7,fmt=str) T(i,0:n+1)
  !end do
  !close(unit=7)

end program laplsolv
