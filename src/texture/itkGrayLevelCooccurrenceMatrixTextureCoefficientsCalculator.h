/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkGrayLevelCooccurrenceMatrixTextureCoefficientsCalculator.h,v $
  Language:  C++
  Date:      $Date: 2008-02-15 12:09:28 $
  Version:   $Revision: 1.2 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkGrayLevelCooccurrenceMatrixTextureCoefficientsCalculator_h
#define __itkGrayLevelCooccurrenceMatrixTextureCoefficientsCalculator_h

#include "itkHistogram.h"
#include "itkMacro.h"

namespace itk {
namespace Statistics {

/** \class GrayLevelCooccurrenceMatrixTextureCoefficientsCalculator 
*  \brief This class computes texture feature coefficients from a grey level
* co-occurrence matrix.
*
* This class computes features that summarize image texture, given a grey level
* co-occurrence matrix (generated by a ScalarImageToGrayLevelCooccurrenceMatrixGenerator
* or related class).  
*
* The features calculated are as follows (where \f$ g(i, j) \f$ is the element in
* cell i, j of a a normalized GLCM):
*
* "Energy" \f$ = f_1 = \sum_{i,j}g(i, j)^2 \f$
*
* "Entropy" \f$ = f_2 = -\sum_{i,j}g(i, j) \log_2 g(i, j)\f$, or 0 if \f$g(i, j) = 0\f$
*
* "Correlation" \f$ = f_3 = \sum_{i,j}\frac{(i - \mu)(j - \mu)g(i, j)}{\sigma^2} \f$
*
* "Difference Moment" \f$= f_4 = \sum_{i,j}\frac{1}{1 + (i - j)^2}g(i, j) \f$
*
* "Inertia" \f$ = f_5 = \sum_{i,j}(i - j)^2g(i, j) \f$ (sometimes called "contrast.")
*
* "Cluster Shade" \f$ = f_6 = \sum_{i,j}((i - \mu) + (j - \mu))^3 g(i, j) \f$
*
* "Cluster Prominence" \f$ = f_7 = \sum_{i,j}((i - \mu) + (j - \mu))^4 g(i, j) \f$
*
* "Haralick's Correlation" \f$ = f_8 = \frac{\sum_{i,j}(i, j) g(i, j) -\mu_t^2}{\sigma_t^2} \f$
* where \f$\mu_t\f$ and \f$\sigma_t\f$ are the mean and standard deviation of the row
* (or column, due to symmetry) sums.
*
* Above, \f$ \mu =  \f$ (weighted pixel average) \f$ = \sum_{i,j}i \cdot g(i, j) = 
* \sum_{i,j}j \cdot g(i, j) \f$ (due to matrix summetry), and
*
* \f$ \sigma =  \f$ (weighted pixel variance) \f$ = \sum_{i,j}(i - \mu)^2 \cdot g(i, j) = 
* \sum_{i,j}(j - \mu)^2 \cdot g(i, j)  \f$  (due to matrix summetry)    
*
* A good texture feature set to use is the Conners, Trivedi and Harlow set: 
* features 1, 2, 4, 5, 6, and 7. There is some correlation between the various
* features, so using all of them at the same time is not necessarialy a good idea.
*
* NOTA BENE: The input histogram will be forcably normalized!
* This algorithm takes three passes through the input
* histogram if the histogram was already normalized, and four if not.
* 
* Web references:
*
* http://www.cssip.uq.edu.au/meastex/www/algs/algs/algs.html
* http://www.ucalgary.ca/~mhallbey/texture/texture_tutorial.html
*
* Print references:
*
* Haralick, R.M., K. Shanmugam and I. Dinstein. 1973.  Textural Features for 
* Image Classification. IEEE Transactions on Systems, Man and Cybernetics. 
* SMC-3(6):610-620.
*
* Haralick, R.M. 1979. Statistical and Structural Approaches to Texture. 
* Proceedings of the IEEE, 67:786-804.
*
* R.W. Conners and C.A. Harlow. A Theoretical Comaprison of Texture Algorithms. 
* IEEE Transactions on Pattern Analysis and Machine Intelligence,  2:204-222, 1980.
*
* R.W. Conners, M.M. Trivedi, and C.A. Harlow. Segmentation of a High-Resolution
* Urban Scene using Texture  Operators. Computer Vision, Graphics and Image 
* Processing, 25:273-310,  1984.
*
* \sa ScalarImageToGrayLevelCooccurrenceMatrixGenerator
* \sa MaskedScalarImageToGrayLevelCooccurrenceMatrixGenerator
* \sa ScalarImageTextureCalculator
*
* Author: Zachary Pincus
*/
    
/** Texture feature types */
enum TextureFeatureName { Energy, Entropy, Correlation,
  InverseDifferenceMoment, Inertia, ClusterShade, ClusterProminence,
  HaralickCorrelation };
    
template< class THistogram >
class GrayLevelCooccurrenceMatrixTextureCoefficientsCalculator : public Object
{
public:
  /** Standard typedefs */
  typedef GrayLevelCooccurrenceMatrixTextureCoefficientsCalculator Self;
  typedef Object                    Superclass;
  typedef SmartPointer<Self>        Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro( GrayLevelCooccurrenceMatrixTextureCoefficientsCalculator, Object );

  /** standard New() method support */
  itkNewMacro( Self ) ;

  /** Typedefs. */
  typedef THistogram                                      HistogramType;
  typedef typename HistogramType::Pointer                 HistogramPointer;
  typedef typename HistogramType::ConstPointer            HistogramConstPointer;
  typedef typename HistogramType::ConstIterator           HistogramConstIterator;
  typedef typename HistogramType::MeasurementType         MeasurementType;
  typedef typename HistogramType::MeasurementVectorType   MeasurementVectorType;
  typedef typename HistogramType::IndexType               IndexType;

  typedef typename HistogramType::RelativeFrequencyType   RelativeFrequencyType;
  typedef typename HistogramType::AbsoluteFrequencyType   AbsoluteFrequencyType;

  /** Triggers the computation of the histogram. */
  virtual void Compute( void );

  /** Connects the GLCM histogram over which the features are going to be computed. */
  itkSetObjectMacro( Histogram, HistogramType );
  itkGetObjectMacro( Histogram, HistogramType );

  /** Methods to return the feature values.
   * \warning These outputs are only valid after the Compute() method has been invoked.
   * \sa Compute()
   */
  double GetFeature( TextureFeatureName feature );
  double GetFeature( unsigned int feature );

  itkGetMacro( Energy, double );
  itkGetMacro( Entropy, double );
  itkGetMacro( Correlation, double );
  itkGetMacro( InverseDifferenceMoment, double );
  itkGetMacro( Inertia, double );
  itkGetMacro( ClusterShade, double );
  itkGetMacro( ClusterProminence, double );
  itkGetMacro( HaralickCorrelation, double );

protected:

  /** Constructor. */
  GrayLevelCooccurrenceMatrixTextureCoefficientsCalculator();

  /** Destructor. */
  virtual ~GrayLevelCooccurrenceMatrixTextureCoefficientsCalculator() {};

  /** PrintSelf. */
  void PrintSelf( std::ostream& os, Indent indent ) const;    

private:

  GrayLevelCooccurrenceMatrixTextureCoefficientsCalculator( const Self& ); // purposely not implemented
  void operator=( const Self& );            // purposely not implemented

  virtual void ResetFeatureValues( void );

  /** The member variables: input histogram. */
  HistogramPointer  m_Histogram;

  /** The member variables: output feature values. */
  double            m_Energy;
  double            m_Entropy;
  double            m_Correlation;
  double            m_InverseDifferenceMoment;
  double            m_Inertia;
  double            m_ClusterShade;
  double            m_ClusterProminence;
  double            m_HaralickCorrelation;

}; // end class GrayLevelCooccurrenceMatrixTextureCoefficientsCalculator


} // end of namespace Statistics 
} // end of namespace itk 

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGrayLevelCooccurrenceMatrixTextureCoefficientsCalculator.txx"
#endif

#endif
