// This file is part of Hermes2D.
//
// Hermes2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Hermes2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hermes2D.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __H2D_EXACT_SOLUTION_H
#define __H2D_EXACT_SOLUTION_H

#include "solution.h"

namespace Hermes
{
  namespace Hermes2D
  {
    /// \brief Represents an exact solution of a PDE.
    ///
    /// ExactSolution represents an arbitrary user-specified function defined on a domain (mesh),
    /// typically an exact solution to a PDE. This can be used to compare an approximate solution
    /// with an exact solution (see DiffFilter).
    template<typename Scalar>
    class HERMES_API ExactSolution : public Solution<Scalar>
    {
    public:
      ExactSolution(Mesh* mesh);

      /// Dimension of result - either 1 or 2.
      virtual unsigned int get_dimension() const = 0;

      /// For scaling of the solution.
      Scalar exact_multiplicator;
    };

    /// These classes are abstract (pure virtual destructor).
    /// The user is supposed to subclass them (see e.g. NIST benchmarks).
    template<typename Scalar>
    class HERMES_API ExactSolutionScalar : public ExactSolution<Scalar>
    {
    public:
      ExactSolutionScalar(Mesh* mesh);

      /// For Scalar-valued solutions this returns 1.
      virtual unsigned int get_dimension() const;

      /// Function returning the value.
      virtual Scalar value (double x, double y) = 0;

      /// Function returning the derivatives.
      virtual void derivatives (double x, double y, Scalar& dx, Scalar& dy) = 0;

      /// Function returning the value and derivatives.
      Scalar exact_function (double x, double y, Scalar& dx, Scalar& dy)  {
        derivatives (x, y, dx, dy);
        return value (x, y);
      };

      /// Function returning the integration order that 
      /// should be used when integrating the function.
      virtual Hermes::Ord ord(Hermes::Ord x, Hermes::Ord y) = 0;
    };

    template<typename Scalar>
    class HERMES_API ExactSolutionVector : public ExactSolution<Scalar>
    {
    public:
      ExactSolutionVector(Mesh* mesh);

      /// For vector-valued solutions this returns 2.
      virtual unsigned int get_dimension() const;

      /// Function returning the value.
      virtual Scalar2<Scalar> value (double x, double y) const = 0;

      /// Function returning the derivatives.
      virtual void derivatives (double x, double y, Scalar2<Scalar>& dx, Scalar2<Scalar>& dy) const = 0;

      /// Function returning the value and derivatives.
      virtual Scalar2<Scalar> exact_function(double x, double y, Scalar2<Scalar>& dx, Scalar2<Scalar>& dy) const {
        derivatives (x, y, dx, dy);
        return value (x, y);
      };

      /// Function returning the integration order that 
      /// should be used when integrating the function.
      virtual Hermes::Ord ord(Hermes::Ord x, Hermes::Ord y) const = 0;
    };

    template<typename Scalar>
    class HERMES_API ConstantSolution : public ExactSolutionScalar<Scalar>
    {
    public:
      ConstantSolution(Mesh* mesh, Scalar constant);

      virtual Scalar value (double x, double y) ;

      virtual void derivatives (double x, double y, double& dx, double& dy) ;

      virtual Ord ord(Ord x, Ord y) ;
    protected:
      Scalar constant;
    };

    class HERMES_API ZeroSolution : public ExactSolutionScalar<double>
    {
    public:
      ZeroSolution(Mesh* mesh);

      virtual double value (double x, double y);

      virtual void derivatives (double x, double y, double& dx, double& dy);

      virtual Ord ord(Ord x, Ord y);
    };
  }
}
#endif
