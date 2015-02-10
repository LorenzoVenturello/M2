// Copyright 2005 Michael E. Stillman.

#include "res-gausser.hpp"
#include "res-f4-mem.hpp"
#include "moninfo.hpp"

ResGausser *ResGausser::newResGausser(const Ring *K, ResF4Mem *Mem0)
{
  const Z_mod *Kp = K->cast_to_Z_mod();
  if (Kp != 0)
    return new ResGausser(Kp,Mem0);
  return 0;
}

ResGausser::ResGausser(const Z_mod *K0, ResF4Mem *Mem0)
  : typ(ZZp), K(K0), Kp(K0->get_CoeffRing()), Mem(Mem0), n_dense_row_cancel(0), n_subtract_multiple(0)
{
}

ResGausser::CoefficientArray ResGausser::from_ringelem_array(int len, ring_elem *elems) const
{
  int i;
  switch (typ) {
  case ZZp:
    int *result = Mem->coefficients.allocate(len);
    for (i=0; i<len; i++)
      result[i] = elems[i].int_val;
    return result;
  };
  return 0;
}

void ResGausser::to_ringelem_array(int len, CoefficientArray F, ring_elem *result) const
{
  int* elems = F;
  int i;
  switch (typ) {
  case ZZp:
    for (i=0; i<len; i++)
      result[i].int_val = elems[i];
  };
}

ResGausser::CoefficientArray ResGausser::copy_CoefficientArray(int len, CoefficientArray F) const
{
  int* elems = F;
  int i;
  switch (typ) {
  case ZZp:
    int *result = Mem->coefficients.allocate(len);
    for (i=0; i<len; i++)
      result[i] = elems[i];
    return result;
  };
  return 0;
}

void ResGausser::deallocate_F4CCoefficientArray(CoefficientArray &F, int len) const
{
  int* elems = F;
  switch (typ) {
  case ZZp:
    Mem->coefficients.deallocate(elems);
    F = 0;
  };
}
/////////////////////////////////////////////////////////////////////
///////// Dense row routines ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void ResGausser::dense_row_allocate(dense_row &r, int nelems) const
{
  int *elems = Mem->coefficients.allocate(nelems);
  r.coeffs = elems;
  r.len = nelems;
  for (int i=0; i<nelems; i++)
    Kp->set_zero(elems[i]);
}

void ResGausser::dense_row_clear(dense_row &r, int first, int last) const
{
  int* elems = r.coeffs;
  for (int i=first; i<=last; i++)
    Kp->set_zero(elems[i]);
}

void ResGausser::dense_row_deallocate(dense_row &r) const
{
  deallocate_F4CCoefficientArray(r.coeffs, r.len);
  r.len = 0;
}

void ResGausser::dense_row_fill_from_sparse(dense_row &r,
                                         int len,
                                         CoefficientArray sparse,
                                         int *comps) const
{
  int* elems = r.coeffs;
  int* sparseelems = sparse;
  for (int i=0; i<len; i++)
    elems[*comps++] = *sparseelems++;

}

int ResGausser::dense_row_next_nonzero(dense_row &r, int first, int last) const
{
  int* elems = r.coeffs;
  elems += first;
  for (int i=first; i<=last; i++)
    if (!Kp->is_zero(*elems++))
      return i;
  return last+1;
}

void ResGausser::dense_row_cancel_sparse(dense_row &r,
                                      int len,
                                      CoefficientArray sparse,
                                      int *comps) const
{
  int* elems = r.coeffs;
  int* sparseelems = sparse;

  // Basically, over ZZ/p, we are doing: r += a*sparse,
  // where sparse is monic, and a is -r.coeffs[*comps].

  n_dense_row_cancel++;
  n_subtract_multiple += len;
  int a = elems[*comps];
  //  for (int i=0; i<len; i++)
  for (int i=len; i>0; i--)
    Kp->subtract_multiple(elems[*comps++], a, *sparseelems++);
}

void ResGausser::dense_row_to_sparse_row(dense_row &r,
                                      int &result_len,
                                      CoefficientArray &result_sparse,
                                      int *&result_comps,
                                      int first,
                                      int last) const
{
  int* elems = r.coeffs;
  int len = 0;
  for (int i=first; i<=last; i++)
    if (!Kp->is_zero(elems[i])) len++;
  int *in_sparse = Mem->coefficients.allocate(len);
  int *in_comps = Mem->components.allocate(len);
  result_len = len;
  result_sparse = in_sparse;
  result_comps = in_comps;
  for (int i=first; i<=last; i++)
    if (!Kp->is_zero(elems[i]))
      {
        *in_sparse++ = elems[i];
        *in_comps++ = i;
        Kp->set_zero(elems[i]);
      }
}

void ResGausser::sparse_row_make_monic(int len,
                                    CoefficientArray sparse) const
{
  int* elems = sparse;
  int lead = *elems;
  // invert lead:
  Kp->invert(lead,lead);
  for (int i=0; i<len; i++, elems++)
    {
      // multiply the non-zero value *elems by lead.
      Kp->mult(*elems, *elems, lead);
    }
}

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// indent-tabs-mode: nil
// End:
