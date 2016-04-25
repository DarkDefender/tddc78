program laplsolv

    use omp_lib
    implicit none

    !-----------------------------------------------------------------------
    ! Serial program for solving the heat conduction problem 
    ! on a square using the Jacobi method. 
    ! Written by Fredrik Berntsson (frber@math.liu.se) March 2003
    ! Modified by Berkant Savas (besav@math.liu.se) April 2006
    !-----------------------------------------------------------------------
    integer, parameter                  :: n=1000, maxiter=1000
    double precision,parameter          :: tol=1.0E-3
    double precision,dimension(0:n+1,0:n+1) :: T
    double precision,dimension(n)       :: tmp1,tmp2
    double precision,dimension(0:n+1)     :: tmp3
    double precision                    :: error,x
    real                                :: t1,t0
    integer                             :: i,j,k
    character(len=20)                   :: str
    integer :: t_tot_no, t_id, chunk_size, start_idx, end_idx
    

    ! Set boundary conditions and initial values for the unknowns
    T=0.0D0
    T(0:n+1 , 0)     = 1.0D0
    T(0:n+1 , n+1)   = 1.0D0
    T(n+1   , 0:n+1) = 2.0D0


    ! Solve the linear system of equations using the Jacobi method
    call cpu_time(t0)

    do k=1,maxiter

        tmp1=T(1:n,0)
        error=0.0D0

        do j=1,n
            tmp2 = T(1:n,j)
            tmp3 = T(0:n+1,j)
            !$omp parallel private(t_id, t_tot_no, start_idx, end_idx)

            !$omp single
            t_tot_no = omp_get_num_threads()
            t_id = omp_get_thread_num()
            !$omp end single

            chunk_size = n / t_tot_no
            
            start_idx = chunk_size * t_id + 1

            if( t_id == t_tot_no - 1) then
                end_idx = n
            else
                end_idx = chunk_size * (t_id + 1)
            end if

            T( start_idx : end_idx,j) = (tmp3(start_idx - 1 : end_idx - 1) + &
                                        tmp3(start_idx + 1 : end_idx + 1) + &
                                        T(start_idx : end_idx,j+1) + &
                                        tmp1(start_idx : end_idx )) / 4.0D0
            
            !T(1:n,j)=(T(0:n-1,j)+T(2:n+1,j)+T(1:n,j+1)+tmp1)/4.0D0
            !$omp end parallel
            error=max(error,maxval(abs(tmp2-T(1:n,j))))
            tmp1=tmp2
        end do

        if (error<tol) then
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
