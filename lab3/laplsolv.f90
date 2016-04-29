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

    n = size(tmp_index)

    i = index

    call get_index(i, tmp_index)

    if (i > -1) then
        return
    end if

    do i = 1,n
       if (tmp_index(i) == -1) then
          tmp_index(i) = index
          tmp_storage(1:size(in_array),i) = in_array
          return
       end if
    end do

    print *,"Failed to find tmp index"
  end subroutine save_temporary

  subroutine get_index(index, tmp_index)
    integer, intent(inout)::index
    integer, intent(in)::tmp_index(:)

    ! local variables
    integer :: i, n

    n = size(tmp_index)

    do i=1,n
       if (index == tmp_index(i)) then
          index = i
          return
       end if
    end do

    index = -1

    !print *, "Didn't find any value in get_index"

  end subroutine get_index

  ! Clean the temporary buffer array
  subroutine clean_temporary(index, tmp_index, finished_list, n)
    integer, intent(in)::index
    logical, intent(inout)::finished_list(:)
    integer, intent(inout)::tmp_index(:)
    integer, intent(in) :: n

    ! local variables
    integer :: i, idx

    ! Adjuest idx because fortran doesn't know that our array starts at 0 outside of this subroutine
    idx = index + 1 

    finished_list(idx) = .true.

    if ( finished_list(idx-1) .and. finished_list(idx+1)) then
        i = index
        call get_index(i, tmp_index)
       tmp_index(i) = -1
    end if

    !Nested ifs because fortran
    if ( index + 2 < n ) then
        if ( finished_list(idx + 1) .and. finished_list(idx + 2)) then
        i = index + 1
        call get_index(i, tmp_index)
        tmp_index(i) = -1
        end if
    end if

    if ( index - 2 >= 0 ) then
        if ( finished_list(idx - 1) .and. finished_list(idx - 2)) then
        i = index - 1
        call get_index(i, tmp_index)
        tmp_index(i) = -1
        end if
    end if

    if ( index == 1 ) then
        i = 0
        call get_index(i, tmp_index)
       tmp_index(i) = -1
   end if

  end subroutine clean_temporary

end module helper_func

program laplsolv

  use omp_lib
  use helper_func
  implicit none

  integer, parameter                  :: n=1000, maxiter=1000
  double precision,parameter          :: tol=1.0E-3
  !double precision,dimension(0:n+1,0:n+1) :: T
  double precision                    :: error,x
  double precision                    :: t1,t0
  integer                             :: i,j,k
  logical,dimension(0:n+1)            :: finished_index
  integer,dimension(:),allocatable   :: temporary_index
  double precision,allocatable       :: temporary_storage(:,:), T(:,:)
  double precision,dimension(:),allocatable   :: local_error
  character(len=20)                   :: str
  integer                             :: tot_threads, t_id, storage_size, id_b, id_a

  ! Initialize all the finnished indicies to false (fortran boolean looks weird)
  finished_index(1:n) = .false.
  finished_index(0) = .true.
  finished_index(n+1) = .true.

  ! Allocate T
  allocate(T(0:n+1,0:n+1))
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
  storage_size = tot_threads*3 + 1
  allocate(temporary_index(storage_size))
  allocate(local_error(0:tot_threads-1))
  allocate(temporary_storage(n,storage_size))

  temporary_index(1:storage_size) = -1

  print *, "Storeage size is:",storage_size

  !call cpu_time(t0)
  t0 = omp_get_wtime()
  do k=1,maxiter
     local_error(0:tot_threads - 1) = 0.0D0
     !$omp parallel private(t_id)
     t_id = omp_get_thread_num()
     !$omp do private(id_a,id_b,i) schedule(dynamic)
     do j=1,n
        !$omp critical
        !print *, j
        call save_temporary(T(1:n,j-1),j-1,temporary_index,temporary_storage)
        call save_temporary(T(1:n,j),j,temporary_index,temporary_storage)
        call save_temporary(T(1:n,j+1),j+1,temporary_index,temporary_storage)
        !$omp end critical
        id_a = j - 1
        id_b = j + 1

        !Get the id for the data above us
        call get_index(id_a,temporary_index)
        !Get the id for the data below us
        call get_index(id_b,temporary_index)
        T(1:n,j)= &
             (T(0:n-1,j)+ &
             T(2:n+1,j)+ &
             temporary_storage(1:n,id_b)+ &
             temporary_storage(1:n,id_a))/4.0D0
        i = j
        call get_index(i,temporary_index)
        local_error( t_id )=max( &
             local_error(t_id), maxval( &
             abs( temporary_storage(1:n,i)-T(1:n,j) &
             )))
        !$omp critical
        call clean_temporary(j,temporary_index,finished_index,n+2)
        !$omp end critical
     end do
     !$omp end do
     !$omp end parallel

     error = maxval(local_error)

     if (error<tol) then
       exit
    end if

     temporary_index(1:storage_size) = -1
     finished_index(1:n) = .false.
  end do

  !call cpu_time(t1)

  t1 = omp_get_wtime()

  write(unit=*,fmt=*) 'Time:',t1-t0,'Number of Iterations:',k
  write(unit=*,fmt=*) 'Temperature of element T(1,1)  =',T(1,1)

  deallocate(temporary_index)
  deallocate(local_error)
  deallocate(temporary_storage)
  deallocate(T)

  ! Uncomment the next part if you want to write the whole solution
  ! to a file. Useful for plotting.

  !open(unit=7,action='write',file='result.dat',status='unknown')
  !write(unit=str,fmt='(a,i6,a)') '(',N,'F10.6)'
  !do i=0,n+1
  !   write (unit=7,fmt=str) T(i,0:n+1)
  !end do
  !close(unit=7)

end program laplsolv
