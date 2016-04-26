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
        print *, i
        call get_index(i, tmp_index)
        print *, i
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
  integer                             :: tot_threads, t_id, storage_size, id_b, id_a

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
  storage_size = tot_threads*2 + 1
  allocate(temporary_index(storage_size))
  allocate(temporary_storage(n,storage_size))

  temporary_index(1:storage_size) = -1

  print *, "Storeage size is:",storage_size

  call cpu_time(t0)

  do k=1,maxiter
     error=0.0D0
     do j=1,n
        call save_temporary(T(1:n,j-1),j-1,temporary_index,temporary_storage)
        call save_temporary(T(1:n,j),j,temporary_index,temporary_storage)
        call save_temporary(T(1:n,j+1),j+1,temporary_index,temporary_storage)
        !Get the id for the data above us
        id_a = j - 1
        call get_index(id_a,temporary_index)
        !Get the id for the data below us
        id_b = j + 1
        call get_index(id_b,temporary_index)
        !print *,i
        T(1:n,j)= &
             (T(0:n-1,j)+ &
             T(2:n+1,j)+ &
             temporary_storage(1:n,id_b)+ &
             temporary_storage(1:n,id_a))/4.0D0
        i = j
        call get_index(i,temporary_index)
        error=max( &
             error, maxval( &
             abs( temporary_storage(1:n,i)-T(1:n,j) &
             )))
        call clean_temporary(j,temporary_index,finished_index,n+2)
     end do


     if (error<tol) then
        exit
     end if
     
     temporary_index(1:storage_size) = -1
     finished_index(1:n) = .false.
  end do

  call cpu_time(t1)

  write(unit=*,fmt=*) 'Time:',t1-t0,'Number of Iterations:',k
  write(unit=*,fmt=*) 'Temperature of element T(1,1)  =',T(1,1)

  deallocate(temporary_index)
  deallocate(temporary_storage)

  ! Uncomment the next part if you want to write the whole solution
  ! to a file. Useful for plotting.

  !open(unit=7,action='write',file='result.dat',status='unknown')
  !write(unit=str,fmt='(a,i6,a)') '(',N,'F10.6)'
  !do i=0,n+1
  !   write (unit=7,fmt=str) T(i,0:n+1)
  !end do
  !close(unit=7)

end program laplsolv
