#ifndef __itkMultiLabelSTAPLE2ImageFilter_h
#define __itkMultiLabelSTAPLE2ImageFilter_h

#include "itkImage.h"
#include "itkImageToImageFilter.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"

#include <vector>
#include "itkArray.h"
#include "itkArray2D.h"

namespace itk
{
  /** \class MultiLabelSTAPLE2ImageFilter
  *
  * \brief This filter performs a pixelwise combination of an arbitrary number
  * of input images, where each of them represents a segmentation of the same
  * scene (i.e., image).
  *
  * The labelings in the images are weighted relative to each other based on
  * their "performance" as estimated by an expectation-maximization
  * algorithm. In the process, a ground truth segmentation is estimated, and
  * the estimated performances of the individual segmentations are relative to
  * this estimated ground truth.
  *
  * The algorithm is based on the MultiLabelSTAPLE algorithm by Rohlfing et al.,
  * which is based on the binary STAPLE algorithm of Warfield et al., as
  * published originally in
  *
  * S. Warfield, K. Zou, W. Wells, "Validation of image segmentation and expert
  * quality with an expectation-maximization algorithm" in MICCAI 2002: Fifth
  * International Conference on Medical Image Computing and Computer-Assisted
  * Intervention, Springer-Verlag, Heidelberg, Germany, 2002, pp. 298-306
  *
  * The multi-label algorithm implemented here is described in detail in
  *
  * T. Rohlfing, D. B. Russakoff, and C. R. Maurer, Jr., "Performance-based
  * classifier combination in atlas-based image segmentation using
  * expectation-maximization parameter estimation," IEEE Transactions on
  * Medical Imaging, vol. 23, pp. 983-994, Aug. 2004.
  *
  * \par INPUTS
  * All input volumes to this filter must be segmentations of an image,
  * that is, they must have discrete pixel values where each value represents
  * a different segmented object.
  *
  * Input volumes must all contain the same size RequestedRegions. Not all
  * input images must contain all possible labels, but all label values must
  * have the same meaning in all images.
  *
  * The filter can optionally be provided with estimates for the a priori class
  * probabilities through the SetPriorProbabilities function. If no estimate is
  * provided, one is automatically generated by analyzing the relative
  * frequencies of the labels in the input images.
  *
  * \par OUTPUTS
  * The filter produces a single output volume. Each output pixel
  * contains the label that has the highest probability of being the correct
  * label, based on the performance models of the individual segmentations.
  * If the maximum probaility is not unique, i.e., if more than one label have
  * a maximum probability, then an "undecided" label is assigned to that output
  * pixel.
  *
  * By default, the label used for undecided pixels is the maximum label value
  * used in the input images plus one. Since it is possible for an image with
  * 8 bit pixel values to use all 256 possible label values, it is permissible
  * to combine 8 bit (i.e., byte) images into a 16 bit (i.e., short) output
  * image.
  *
  * In addition to the combined image, the estimated confusion matrices for
  * each of the input segmentations can be obtained through the
  * GetConfusionMatrix member function.
  *
  * \par PARAMETERS
  * The label used for "undecided" labels can be set using
  * SetLabelForUndecidedPixels. This functionality can be unset by calling
  * UnsetLabelForUndecidedPixels.
  *
  * A termination threshold for the EM iteration can be defined by calling
  * SetTerminationUpdateThreshold. The iteration terminates once no single
  * parameter of any confusion matrix changes by less than this
  * threshold. Alternatively, a maximum number of iterations can be specified
  * by calling SetMaximumNumberOfIterations. The algorithm may still terminate
  * after a smaller number of iterations if the termination threshold criterion
  * is satisfied.
  *
  * \par EVENTS
  * This filter invokes IterationEvent() at each iteration of the E-M
  * algorithm. Setting the AbortGenerateData() flag will cause the algorithm to
  * halt after the current iteration and produce results just as if it had
  * converged. The algorithm makes no attempt to report its progress since the
  * number of iterations needed cannot be known in advance.
  *
  * This code is largely based on the MultiLabelSTAPLEImageFilter code
  * written by Rohlfing.
  *
  * \author Stefan Klein
  */
  template <typename TInputImage, typename TOutputImage = TInputImage,
    typename TWeights = float >
  class MultiLabelSTAPLE2ImageFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
  {
  public:
    /** Standard class typedefs. */
    typedef MultiLabelSTAPLE2ImageFilter Self;
    typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self>  ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods) */
    itkTypeMacro(MultiLabelSTAPLE2ImageFilter, ImageToImageFilter);

    /** Extract some information from the image types.  Dimensionality
    * of the two images is assumed to be the same. */
    typedef typename TOutputImage::PixelType OutputPixelType;
    typedef typename TInputImage::PixelType InputPixelType;

    /** Extract some information from the image types.  Dimensionality
    * of the two images is assumed to be the same. */
    itkStaticConstMacro(ImageDimension, unsigned int,
      TOutputImage::ImageDimension);

    /** Image typedef support */
    typedef TInputImage  InputImageType;
    typedef TOutputImage OutputImageType;
    typedef typename InputImageType::Pointer InputImagePointer;
    typedef typename OutputImageType::Pointer OutputImagePointer;

    /** Superclass typedefs. */
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    /** Confusion matrix typedefs. */
    typedef TWeights WeightsType;
    typedef Array2D<WeightsType> ConfusionMatrixType;

    /** Typedefs for prior probabilities */
    typedef Array<WeightsType>                      PriorProbabilitiesType;
    typedef Image<
      WeightsType,
      ::itk::GetImageDimension<
        InputImageType>::ImageDimension>            ProbabilityImageType;
    typedef typename ProbabilityImageType::Pointer  ProbabilityImagePointer;
    typedef std::vector<ProbabilityImagePointer>    PriorProbabilityImageArrayType;

    /** Various typedefs */
    typedef Array<WeightsType>                      ObserverTrustType;
    typedef std::vector<ProbabilityImagePointer>    ProbabilisticSegmentationArrayType;
    typedef Array<OutputPixelType>                  PriorPreferenceType;

    /** Typedefs for mask support */
    typedef InputImageType                          MaskImageType;
    typedef typename MaskImageType::Pointer         MaskImagePointer;
    typedef typename MaskImageType::PixelType       MaskPixelType;

    /** Iterator types. */
    typedef ImageRegionConstIterator< InputImageType >  InputConstIteratorType;
    typedef ImageRegionIterator< OutputImageType >      OutputIteratorType;
    typedef ImageRegionConstIterator<
      ProbabilityImageType >                            ProbConstIteratorType;
    typedef ImageRegionIterator<
      ProbabilityImageType >                            ProbIteratorType;
    typedef ImageRegionConstIterator< MaskImageType >   MaskConstIteratorType;

    /** Set/get/unset maximum number of iterations. */
    virtual void SetMaximumNumberOfIterations( const unsigned int mit )
    {
      this->m_MaximumNumberOfIterations = mit;
      this->m_HasMaximumNumberOfIterations = true;
      this->Modified();
    }

    itkGetConstMacro(MaximumNumberOfIterations, unsigned int);

    virtual void UnsetMaximumNumberOfIterations()
    {
      if ( this->m_HasMaximumNumberOfIterations )
      {
        this->m_HasMaximumNumberOfIterations = false;
        this->Modified();
      }
    }

    /** Set/get termination threshold. Convergence is assumed when all confusion
     * matrix element updates are lower than this threshold. */
    itkSetMacro( TerminationUpdateThreshold, WeightsType );
    itkGetConstMacro( TerminationUpdateThreshold, WeightsType );

    /** Set/get/unset prior preference; a scalar for each class indicating
     * the preference in case of undecided pixels. The lower the number,
     * the more preference. If not provided, the class numbers are assumed
     * as preferences. Make sure no duplicate values exist, and no
     * numbers higher than the numberOfClasses-1. */
    virtual void SetPriorPreference( const PriorPreferenceType& ppa )
    {
      this->m_PriorPreference = ppa;
      this->m_HasPriorPreference = true;
      this->Modified();
    }

    itkGetConstReferenceMacro( PriorPreference, PriorPreferenceType );

    virtual void UnsetPriorPreference( void )
    {
      if ( this->m_HasPriorPreference )
      {
        this->m_HasPriorPreference = false;
        this->Modified();
      }
    }

    /** Set/get/unset an array of prior probability images */
    virtual void SetPriorProbabilityImageArray( const PriorProbabilityImageArrayType & arg)
    {
      this->m_PriorProbabilityImageArray = arg;
      this->m_HasPriorProbabilityImageArray = true;
      this->Modified();
    }

    virtual const PriorProbabilityImageArrayType & GetPriorProbabilityImageArray( void ) const
    {
      return this->m_PriorProbabilityImageArray;
    }

    virtual void UnsetPriorProbabilityImageArray( void )
    {
      if ( this->m_HasPriorProbabilityImageArray )
      {
        this->m_HasPriorProbabilityImageArray = false;
        this->Modified();
      }
    }

    /** Set/get/unset prior class probabilities; a scalar for each class. */
    virtual void SetPriorProbabilities( const PriorProbabilitiesType& ppa )
    {
      this->m_PriorProbabilities = ppa;
      this->m_HasPriorProbabilities = true;
      this->Modified();
    }

    itkGetConstReferenceMacro( PriorProbabilities, PriorProbabilitiesType );

    virtual void UnsetPriorProbabilities( void )
    {
      if ( this->m_HasPriorProbabilities )
      {
        this->m_HasPriorProbabilities = false;
        this->Modified();
      }
    }

    /** Set/get/unset observer trust factors.  */
    virtual void SetObserverTrust( const ObserverTrustType& ot )
    {
      this->m_ObserverTrust = ot;
      this->m_HasObserverTrust = true;
      this->Modified();
    }

    itkGetConstReferenceMacro( ObserverTrust, ObserverTrustType);

    virtual void UnsetObserverTrust( void )
    {
      if ( this->m_HasObserverTrust )
      {
        this->m_HasObserverTrust = false;
        this->Modified();
      }
    }

    /** Set/unset/get the number of classes. If you don't set it, it is
     * automatically determined from the input segmentations */
    virtual void SetNumberOfClasses(InputPixelType arg)
    {
      this->m_NumberOfClasses = arg;
      this->m_HasNumberOfClasses = true;
      this->Modified();
    }

    virtual void UnsetNumberOfClasses( void )
    {
      if ( this->m_HasNumberOfClasses )
      {
        this->m_HasNumberOfClasses = false;
        this->Modified();
      }
    }

    itkGetConstMacro( NumberOfClasses, InputPixelType );

    /** Set/Get a mask image; If a mask is supplied, only pixels that are
     * within the mask are used in the staple procedure. The output
     * at pixels outside the mask will be equal to that of the first
     * observer */
    itkSetObjectMacro( MaskImage, MaskImageType );
    itkGetObjectMacro( MaskImage, MaskImageType );

    /** Set whether a majority voting step should be used
     * to initialize the confusion matrix */
    itkSetMacro( InitializeWithMajorityVoting, bool)
    itkGetConstMacro( InitializeWithMajorityVoting, bool)

    /** Setting: turn on/off to whether a probabilistic segmentation
     * is generated; default: false */
    itkSetMacro(GenerateProbabilisticSegmentations, bool);
    itkGetConstMacro(GenerateProbabilisticSegmentations, bool);

    /** Get the probabilistic segmentations. Only valid when
     * SetGenerateProbabilisticSegmentations(true) has been
     * invoked before updating this filter. */
    virtual const ProbabilisticSegmentationArrayType &
      GetProbabilisticSegmentationArray( void ) const
    {
      return this->m_ProbabilisticSegmentationArray;
    }

    /** If you have inspected the probabilistic segmentations and want to get rid
     * of those float images sitting in your memory, call this function */
    virtual void CleanProbabilisticSegmentations( void )
    {
      if ( this->m_ProbabilisticSegmentationArray.size() > 0 )
      {
        this->m_ProbabilisticSegmentationArray =
          ProbabilisticSegmentationArrayType(0);
        this->Modified();
      }
    }

    /** The maximum confusion matrix element update in the last iteration */
    itkGetConstMacro( MaximumConfusionMatrixElementUpdate, WeightsType );

    /** Get confusion matrix for the i-th input segmentation. */
    virtual const ConfusionMatrixType & GetConfusionMatrix( const unsigned int i ) const
    {
      return this->m_ConfusionMatrixArray[i];
    }

    /** Get the number of elapsed iterations */
    itkGetConstMacro( ElapsedIterations, unsigned int );


  protected:
    /** Constructor */
    MultiLabelSTAPLE2ImageFilter();

    /** Destructor */
    virtual ~MultiLabelSTAPLE2ImageFilter() {}

    /** Do the actual work */
    void GenerateData();

    /** Print some information, not really implemented */
    void PrintSelf(std::ostream&, Indent) const;

    /** Determine maximum value among all input images' pixels */
    virtual InputPixelType ComputeMaximumInputValue();

    /** Initialize the prior probabilities, if not supplied by the user */
    virtual void InitializePriorProbabilities();

    /** Initialize the confusion matrices */
    virtual void AllocateConfusionMatrixArray();
    virtual void InitializeConfusionMatrixArray();

    /** The number of different labels found in the input segmentations */
    InputPixelType m_NumberOfClasses;

    /** Variables that store whether the a specific parameter has been
     * set by the user */
    bool m_HasPriorProbabilities;
    bool m_HasObserverTrust;
    bool m_HasMaximumNumberOfIterations;
    bool m_HasPriorProbabilityImageArray;
    bool m_HasNumberOfClasses;
    bool m_HasPriorPreference;

    /** These variables could in principle be accessed via the member functions,
     * but for inheriting classes this would be annoying. So, make them protected. */
    PriorProbabilitiesType             m_PriorProbabilities;
    PriorProbabilityImageArrayType     m_PriorProbabilityImageArray;
    ObserverTrustType                  m_ObserverTrust;
    std::vector<ConfusionMatrixType>   m_ConfusionMatrixArray;
    std::vector<ConfusionMatrixType>   m_UpdatedConfusionMatrixArray;
    ProbabilisticSegmentationArrayType m_ProbabilisticSegmentationArray;
    PriorPreferenceType                m_PriorPreference;

    /** Variables updated during iterating: */
    WeightsType m_MaximumConfusionMatrixElementUpdate;
    unsigned int m_ElapsedIterations;

  private:
    MultiLabelSTAPLE2ImageFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

    /** Settings that can be accessed via the set/get member functions */
    unsigned int m_MaximumNumberOfIterations;
    bool m_GenerateProbabilisticSegmentations;
    WeightsType m_TerminationUpdateThreshold;
    MaskImagePointer m_MaskImage;
    bool m_InitializeWithMajorityVoting;


  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiLabelSTAPLE2ImageFilter.txx"
#endif

#endif
