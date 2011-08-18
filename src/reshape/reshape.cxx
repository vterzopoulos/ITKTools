/*=========================================================================
*
* Copyright Marius Staring, Stefan Klein, David Doria. 2011.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0.txt
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*=========================================================================*/
/** \file
 \brief Reshape an image.
 
 \verbinclude reshape.help
 */
#include "itkCommandLineArgumentParser.h"
#include "ITKToolsHelpers.h"
#include "ITKToolsBase.h"

#include "itkImageFileReader.h"
#include "itkReshapeImageToImageFilter.h"
#include "itkImageFileWriter.h"
#include <itksys/SystemTools.hxx>


/**
 * ******************* GetHelpString *******************
 */

std::string GetHelpString( void )
{
  std::stringstream ss;
  ss << "Usage:" << std::endl
  << "pxpca" << std::endl
  << "  -in      inputFilename" << std::endl
  << "  [-out]   outputFileName, default inputFileName_reshaped" << std::endl
  << "  -s       size of the output image" << std::endl
  << "Supported: 2D, 3D, (unsigned) char, (unsigned) short, (unsigned) int, (unsigned) long, float, double.";

  return ss.str();

} // end GetHelpString()



class ITKToolsReshapeBase : public itktools::ITKToolsBase
{ 
public:
  ITKToolsReshapeBase()
  {
    this->m_InputFileName = "";
    this->m_OutputFileName = "";
    //std::vector<unsigned long> this->m_OutputSize;
  };
  ~ITKToolsReshapeBase(){};

  /** Input parameters */
  std::string m_InputFileName;
  std::string m_OutputFileName;
  std::vector<unsigned long> m_OutputSize;

    
}; // end ReshapeBase


template< class TComponentType, unsigned int VDimension >
class ITKToolsReshape : public ITKToolsReshapeBase
{
public:
  typedef ITKToolsReshape Self;

  ITKToolsReshape(){};
  ~ITKToolsReshape(){};

  static Self * New( itktools::ComponentType componentType, unsigned int dim )
  {
    if ( itktools::IsType<TComponentType>( componentType ) && VDimension == dim )
    {
      return new Self;
    }
    return 0;
  }

  void Run( void )
  {
    /** Typedefs. */
    typedef itk::Image<TComponentType, VDimension>      ImageType;
    typedef itk::ImageFileReader< ImageType >           ReaderType;
    typedef itk::ReshapeImageToImageFilter< ImageType > ReshapeFilterType;
    typedef itk::ImageFileWriter< ImageType >           WriterType;
    typedef typename ReshapeFilterType::SizeType        SizeType;

    /** Translate vector to SizeType. */
    SizeType size;
    for ( unsigned int i = 0; i < this->m_OutputSize.size(); ++i )
    {
      size[ i ] = this->m_OutputSize[ i ];
    }


    /** Reader. */
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName( this->m_InputFileName.c_str() );

    /** Reshaper. */
    typename ReshapeFilterType::Pointer reshaper = ReshapeFilterType::New();
    reshaper->SetInput( reader->GetOutput() );
    reshaper->SetOutputSize( size );
    reshaper->Update();

    /** Writer. */
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( this->m_OutputFileName.c_str() );
    writer->SetInput( reshaper->GetOutput() );
    writer->Update();
  }

}; // end Reshape

//-------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
  /** Create a command line argument parser. */
  itk::CommandLineArgumentParser::Pointer parser = itk::CommandLineArgumentParser::New();
  parser->SetCommandLineArguments( argc, argv );
  parser->SetProgramHelpText( GetHelpString() );

  parser->MarkArgumentAsRequired( "-in", "The input filename." );
  parser->MarkArgumentAsRequired( "-s", "Output size." );

  itk::CommandLineArgumentParser::ReturnValue validateArguments = parser->CheckForRequiredArguments();

  if( validateArguments == itk::CommandLineArgumentParser::FAILED )
  {
    return EXIT_FAILURE;
  }
  else if( validateArguments == itk::CommandLineArgumentParser::HELPREQUESTED )
  {
    return EXIT_SUCCESS;
  }

  /** Get arguments. */
  std::string inputFilename = "";
  parser->GetCommandLineArgument( "-in", inputFilename );

  std::string base = itksys::SystemTools::GetFilenameWithoutLastExtension(
    inputFilename );
  std::string ext  = itksys::SystemTools::GetFilenameLastExtension(
    inputFilename );
  std::string outputFilename = base + "_reshaped" + ext;
  parser->GetCommandLineArgument( "-out", outputFilename );

  std::vector<unsigned long> outputSize;
  parser->GetCommandLineArgument( "-s", outputSize );

  /** Determine image properties. */
  std::string ComponentTypeIn = "short";
  std::string PixelType; //we don't use this
  unsigned int Dimension = 3;
  unsigned int NumberOfComponents = 1;
  std::vector<unsigned int> inputSize( Dimension, 0 );
  int retgip = itktools::GetImageProperties(
    inputFilename,
    PixelType,
    ComponentTypeIn,
    Dimension,
    NumberOfComponents,
    inputSize );
  if ( retgip != 0 )
  {
    return 1;
  }

  /** Check for vector images. */
  if ( NumberOfComponents > 1 )
  {
    std::cerr << "ERROR: The NumberOfComponents is larger than 1!" << std::endl;
    std::cerr << "  Vector images are not supported." << std::endl;
    return 1;
  }

  /** Get rid of the possible "_" in ComponentType. */
  itktools::ReplaceUnderscoreWithSpace( ComponentTypeIn );

  /** Check dimensions. */
  if ( inputSize.size() != outputSize.size() )
  {
    std::cerr << "ERROR: input and output dimension should be the same.\n";
    std::cerr << "  Please, specify only " << Dimension
      << "numbers with \"-s\"." << std::endl;
    return 1;
  }


  /** Class that does the work */
  ITKToolsReshapeBase * reshape = 0; 

  /** Short alias */
  unsigned int imageDimension = Dimension;
 
  /** \todo some progs allow user to override the pixel type, 
   * so we need a method to convert string to EnumComponentType */
  itktools::ComponentType componentType = itktools::GetImageComponentType(inputFilename);
  
  std::cout << "Detected component type: " << 
    componentType << std::endl;
    
  try
  {    
    if (!reshape) reshape = ITKToolsReshape< unsigned char, 2 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< char, 2 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< unsigned short, 2 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< short, 2 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< unsigned int, 2 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< int, 2 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< unsigned long, 2 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< long, 2 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< float, 2 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< double, 2 >::New( componentType, imageDimension );
#ifdef ITKTOOLS_3D_SUPPORT
    if (!reshape) reshape = ITKToolsReshape< unsigned char, 3 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< char, 3 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< unsigned short, 3 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< short, 3 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< unsigned int, 3 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< int, 3 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< unsigned long, 3 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< long, 3 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< float, 3 >::New( componentType, imageDimension );
    if (!reshape) reshape = ITKToolsReshape< double, 3 >::New( componentType, imageDimension );
#endif
    if (!reshape) 
    {
      std::cerr << "ERROR: this combination of pixeltype and dimension is not supported!" << std::endl;
      std::cerr
        << "pixel (component) type = " << componentType
        << " ; dimension = " << Dimension
        << std::endl;
      return 1;
    }

    reshape->m_InputFileName = inputFilename;
    reshape->m_OutputFileName = outputFilename;
    reshape->m_OutputSize = outputSize;

    reshape->Run();
    
    delete reshape;  
  }
  catch( itk::ExceptionObject &e )
  {
    std::cerr << "Caught ITK exception: " << e << std::endl;
    delete reshape;
    return 1;
  }
  
  /** End program. */
  return 0;

} // end main()
