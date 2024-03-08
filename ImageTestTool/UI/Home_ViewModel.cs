using ImageTestTool.ViewModelHelper;
using Microsoft.Win32;
using RootTools_CLR;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media.Imaging;

namespace ImageTestTool
{
    public class Home_ViewModel : ObservableObject
    {

        // Cpp_ImageProcessParam imageParam = new Cpp_ImageProcessParam();
        public Home_ViewModel()
        {
            // 이미지 처리 함수 초기화 
            InitializeImageProcessingFunctions();
            p_SelectedImageProcess = ImageProcessingFunction.Threshold;
        }

        #region Property
        public enum ImageProcessingFunction
        {
            Threshold,
            TemplateMatching,
            Labeling,
            GaussianBlur,
            AverageBlur,
            MedianBlur,
            Morphology
        }

        public enum ProcessingTechnique
        {
            IPP,
            OpenCV,
            SIMD,
            Loop
        }

        public class ImageProcessingCapabilities
        {
            public static Dictionary<ImageProcessingFunction, List<ProcessingTechnique>> Capabilities = new Dictionary<ImageProcessingFunction, List<ProcessingTechnique>>()
            {
                { ImageProcessingFunction.Threshold, new List<ProcessingTechnique>() { ProcessingTechnique.IPP, ProcessingTechnique.OpenCV, ProcessingTechnique.SIMD, ProcessingTechnique.Loop } },
                { ImageProcessingFunction.TemplateMatching, new List<ProcessingTechnique>() { ProcessingTechnique.IPP, ProcessingTechnique.OpenCV, ProcessingTechnique.Loop } },
                { ImageProcessingFunction.Labeling, new List<ProcessingTechnique>() { ProcessingTechnique.OpenCV, ProcessingTechnique.SIMD } },
                { ImageProcessingFunction.GaussianBlur, new List<ProcessingTechnique>() { ProcessingTechnique.IPP, ProcessingTechnique.OpenCV } },
                { ImageProcessingFunction.AverageBlur, new List<ProcessingTechnique>() { ProcessingTechnique.IPP, ProcessingTechnique.OpenCV } },
                { ImageProcessingFunction.MedianBlur, new List<ProcessingTechnique>() { ProcessingTechnique.IPP, ProcessingTechnique.OpenCV } },
                { ImageProcessingFunction.Morphology, new List<ProcessingTechnique>() { ProcessingTechnique.IPP, ProcessingTechnique.OpenCV } }
            };
        }
        private ObservableCollection<ImageProcessingFunction> m_ImageProcessList;
        public ObservableCollection<ImageProcessingFunction> p_ImageProcessList
        {
            get { return m_ImageProcessList; }
            set { SetProperty(ref m_ImageProcessList, value); }
        }

        private ImageProcessingFunction m_SelectedImageProcess;
        public ImageProcessingFunction p_SelectedImageProcess
        {
            get { return m_SelectedImageProcess; }
            set
            {
                SetProperty(ref m_SelectedImageProcess, value);
                UpdateProcessingTechniques();
            }
        }

        private ObservableCollection<ProcessingTechnique> m_ProcessTechList;
        public ObservableCollection<ProcessingTechnique> p_ProcessTechList
        {
            get { return m_ProcessTechList; }
            set { SetProperty(ref m_ProcessTechList, value); }
        }

        private string m_SelectedProcessTech;
        public string p_SelectedProcessTech
        {
            get { return m_SelectedProcessTech; }
            set { SetProperty(ref m_SelectedProcessTech, value); }
        }

        private string m_ImageFilePath;
        public string p_ImageFilePath
        {
            get { return m_ImageFilePath; }
            set
            {
                if (SetProperty(ref m_ImageFilePath, value))
                {
                    OnPropertyChanged(new PropertyChangedEventArgs("ImageSource"));
                }
            }
        }

        private BitmapImage m_RightImage;
        public BitmapImage p_RightImage
        {
            get { return m_RightImage; }
            set { SetProperty(ref m_RightImage, value); }
        }
        #endregion

        #region RelayCommand
        public ICommand btnImageOpenCommand
        {
            get
            {
                return new RelayCommand(OpenImage);
            }
        }

        public ICommand btnRunCommand
        {
            get
            {
                return new RelayCommand(RunImageProcessing, CanRunImageProcessing);
            }
        }

        #endregion

        public BitmapSource ImageSource
        {
            get
            {
                if (string.IsNullOrEmpty(p_ImageFilePath))
                    return null;

                BitmapImage bitmap = new BitmapImage();
                bitmap.BeginInit();
                bitmap.UriSource = new Uri(p_ImageFilePath);
                bitmap.EndInit();
                return bitmap;
            }
        }

        private void InitializeImageProcessingFunctions()
        {
            // 이미지 처리 함수 목록 초기화
            p_ImageProcessList = new ObservableCollection<ImageProcessingFunction>
            {
                ImageProcessingFunction.Threshold,
                ImageProcessingFunction.TemplateMatching,
                ImageProcessingFunction.Labeling,
                ImageProcessingFunction.GaussianBlur,
                ImageProcessingFunction.AverageBlur,
                ImageProcessingFunction.MedianBlur,
                ImageProcessingFunction.Morphology
            };
        }

        private void UpdateProcessingTechniques()
        {
            // 선택된 이미지 처리 함수에 따라 Tech 업데이트
            p_ProcessTechList = new ObservableCollection<ProcessingTechnique>(ImageProcessingCapabilities.Capabilities[p_SelectedImageProcess]);
        }

        // 이미지 오픈 버튼 커맨드 실행 메서드
        private void OpenImage(object parameter)
        {
            // 나중에 bmp만 선택가능하게 바꿔야할듯
            var openFileDialog = new OpenFileDialog();
            openFileDialog.Filter = "Image Files (*.jpg;*.jpeg;*.png;*.gif;*.bmp)|*.jpg;*.jpeg;*.png;*.gif;*.bmp|All files (*.*)|*.*";
            if (openFileDialog.ShowDialog() == true)
            {
                // 이미지 파일을 바이트 배열로 읽어들임
                byte[] imageData = LoadImageFromFile(openFileDialog.FileName);

                // pSrc에 이미지 데이터 할당
                if (imageData != null)
                {
                    /*imageParam.pSrc = imageData;
                    imageParam.nSrcW = 500; 
                    imageParam.nSrcH = 500; */

                    // 나중에 필요한 이미지 처리 함수 호출해야함
                }

                p_ImageFilePath = openFileDialog.FileName;
            }
        }

        private byte[] LoadImageFromFile(string filePath)
        {
            byte[] imageData = null;
            try
            {
                imageData = File.ReadAllBytes(filePath);
            }
            catch (Exception ex)
            {
                MessageBox.Show("이미지 로드 중 오류 발생: " + ex.Message);
            }
            return imageData;
        }

        private void RunImageProcessing(object parameter)
        {
            switch (p_SelectedImageProcess)
            {
                case ImageProcessingFunction.Threshold:
                    Cpp_ThresholdParam thresholdParam = new Cpp_ThresholdParam();
                    thresholdParam.processMode = p_SelectedProcessTech.ToString();
                    /*thresholdParam.pSrc = imageParam.pSrc;
                    thresholdParam.nSrcW = imageParam.nSrcW;
                    thresholdParam.nSrcH = imageParam.nSrcH;*/

                    thresholdParam.bDark = true; 
                    thresholdParam.nThresh = 128; 

                    // 처리한 이미지 저장 
                    byte[] processedImage = new byte[thresholdParam.nSrcW * thresholdParam.nSrcH];
                    thresholdParam.pDst = processedImage;

                    CLR_IP.Cpp_Threshold(thresholdParam);
                    ShowProcessedImage(processedImage, thresholdParam.nSrcW, thresholdParam.nSrcH);
                    break;
                // 나중에 다른 이미지 처리 함수에 대한 case 추가

                default:
                    MessageBox.Show("이미지 처리 방식 재선택 필요");
                    break;
            }

            MessageBox.Show("처리중");
        }
        private void ShowProcessedImage(byte[] processedImage, int width, int height)
        {
            try
            {
                // 처리된 이미지를 BitmapImage로 변환
                BitmapImage processedBitmap = new BitmapImage();
                processedBitmap.BeginInit();
                processedBitmap.StreamSource = new MemoryStream(processedImage);
                processedBitmap.DecodePixelWidth = width;
                processedBitmap.DecodePixelHeight = height; 
                processedBitmap.EndInit();

                // 오른쪽 이미지 뷰어에 이미지 표시
                p_RightImage = processedBitmap;
            }
            catch (Exception ex)
            {
                MessageBox.Show("처리된 이미지 로드 중 오류 발생: " + ex.Message);
            }
        }
        private bool CanRunImageProcessing(object parameter)
        {
            return !string.IsNullOrEmpty(p_ImageFilePath);
        }
    }
}
