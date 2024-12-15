#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <msclr/marshal_cppstd.h>
#using <System.Drawing.dll>

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;
using namespace System::Collections::Generic;

namespace GraphPlotter {
    public ref class GraphForm : public Form
    {
    public:
        GraphForm()
        {
            InitializeComponent();
        }

    private:
        List<PointF>^ points;
        bool isLineGraph = true;
        Color graphColor = Color::Blue;

        /*
        * Draw all the components of the Form (buttons, texts)
        */
        void InitializeComponent()
        {
            this->Text = "Grafiko atvaizdavimas";
            this->Size = System::Drawing::Size(900, 600);
            this->BackColor = SystemColors::Control;

            Button^ loadButton = gcnew Button();
            loadButton->Text = "Užkrauti faila";
            loadButton->Location = Point(10, 10);
            loadButton->Width = 90;
            loadButton->Click += gcnew EventHandler(this, &GraphForm::LoadCSV);

            Button^ lineGraphButton = gcnew Button();
            lineGraphButton->Text = "Linijinis";
            lineGraphButton->Location = Point(110, 10);
            lineGraphButton->Click += gcnew EventHandler(this, &GraphForm::SetLineGraph);

            Button^ barGraphButton = gcnew Button();
            barGraphButton->Text = "Stulpelinis";
            barGraphButton->Location = Point(210, 10);
            barGraphButton->Click += gcnew EventHandler(this, &GraphForm::SetBarGraph);

            Button^ colorButton = gcnew Button();
            colorButton->Text = "Pakeisti spalva";
            colorButton->Width = 100;
            colorButton->Location = Point(310, 10);
            colorButton->Click += gcnew EventHandler(this, &GraphForm::ChangeColor);

            this->Controls->Add(loadButton);
            this->Controls->Add(lineGraphButton);
            this->Controls->Add(barGraphButton);
            this->Controls->Add(colorButton);

            this->Paint += gcnew PaintEventHandler(this, &GraphForm::OnPaint);
            points = gcnew List<PointF>();
        }

        /*
        * Load the CSV file, read it and form the array of the X and Y coordinates
        */
        void LoadCSV(Object^ sender, EventArgs^ e)
        {
            OpenFileDialog^ openFileDialog = gcnew OpenFileDialog();
            openFileDialog->Filter = "CSV files (*.csv)|*.csv|All files (*.*)|*.*"; // Open File dialog that lets to select csv files (optional all files)

            if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
            {
                String^ fileName = openFileDialog->FileName;
                points->Clear(); // Clear the array of the X and Y coordinates

                std::ifstream file(msclr::interop::marshal_as<std::string>(fileName)); // Open the CSV file
                std::string line;

                while (std::getline(file, line)) // Read the file information
                {
                    std::stringstream reader(line); // Read the line and register it as a cin
                    float x, y;
                    char comma;
                    reader >> x >> comma >> y; // Read X point, space character, Y point
                    points->Add(PointF(x, y)); // Insert X and Y coordinates to the array
                }
                file.close(); // Close the opened file

                this->Invalidate(); // Redraw the graph
            }
        }

        void SetLineGraph(Object^ sender, EventArgs^ e)
        {
            isLineGraph = true; // Set the graph to linear
            this->Invalidate(); // Redraw the graph
        }

        void SetBarGraph(Object^ sender, EventArgs^ e)
        {
            isLineGraph = false; // Set the graph to column
            this->Invalidate(); // Redraw the graph
        }

        void ChangeColor(Object^ sender, EventArgs^ e)
        {
            ColorDialog^ colorDialog = gcnew ColorDialog(); // Open the color selector
            if (colorDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
            {
                graphColor = colorDialog->Color; // Set graph color
                this->Invalidate(); // Redraw the graph
            }
        }

        /*
        * Draw the full graphic with the axis, axis points, and X and Y points using the array of coordinates from the CSV file
        */
        void OnPaint(Object^ sender, PaintEventArgs^ e)
        {
            Graphics^ g = e->Graphics;
            Pen^ axisPen = gcnew Pen(Color::Black, 2);
            Pen^ graphPen = gcnew Pen(graphColor, 2);

            System::Drawing::Font^ axisFont = gcnew System::Drawing::Font("Arial", 10);
            SolidBrush^ textBrush = gcnew SolidBrush(Color::Black);

            int padding = 50;
            RectangleF drawingArea = RectangleF(padding, padding, this->ClientSize.Width - 2 * padding, this->ClientSize.Height - 2 * padding);

            if (points->Count > 0)
            {
                float xMin = points[0].X;
                float xMax = points[0].X;
                float yMin = points[0].Y;
                float yMax = points[0].Y;

                /*
                * Find the min and max value of Y and X points
                */
                for each (PointF point in points)
                {
                    xMin = Math::Min(xMin, point.X);
                    xMax = Math::Max(xMax, point.X);
                    yMin = Math::Min(yMin, point.Y);
                    yMax = Math::Max(yMax, point.Y);
                }

                if (xMin > 0) xMin = 0; // Ensure that X axis would start from 0

                if (yMin > 0) yMin = 0; // Ensure that Y axis would start from 0

                float xScale = drawingArea.Width / (xMax - xMin); // Add scale to the graphic, that all the values would fit in the drawing area
                float yScale = drawingArea.Height / (yMax - yMin); // Add scale to the graphic, that all the values would fit in the drawing area

                PointF origin;
                origin.X = (0.0f >= xMin && 0.0f <= xMax)
                    ? (0.0f - xMin) * xScale + drawingArea.Left
                    : drawingArea.Left; // Fix the values for the X axis line

                origin.Y = drawingArea.Bottom - (0.0f - yMin) * yScale; // Fix the values for the Y axis line

                g->DrawLine(axisPen, origin.X, drawingArea.Top, origin.X, drawingArea.Bottom); // Draw the X axis line

                g->DrawLine(axisPen, drawingArea.Left, origin.Y, drawingArea.Right, origin.Y); // Draw the Y axis line

                float xStep = (xMax - xMin) / 10; // Set the step of the X axis values to be displayed 
                float yStep = (yMax - yMin) / 10; // Set the step of the Y axis values to be displayed 

                /*
                * Draw the X axis labels
                */
                for (float x = xMin; x <= xMax; x += xStep)
                {
                    PointF transformed((x - xMin) * xScale + drawingArea.Left, origin.Y);
                    g->DrawString(Math::Round(x).ToString(), axisFont, textBrush, transformed.X, origin.Y + 5);
                }

                /*
                * Ensure to always draw the 0 value on axis
                */
                if (xMin <= 0 && xMax >= 0)
                {
                    PointF zeroPos((0.0f - xMin) * xScale + drawingArea.Left, origin.Y);
                    g->DrawString("0", axisFont, textBrush, zeroPos.X, origin.Y + 5);
                }

                /*
                * Draw the Y axis labels
                */
                for (float y = yMin; y <= yMax; y += yStep)
                {
                    PointF transformed(origin.X, drawingArea.Bottom - (y - yMin) * yScale);
                    g->DrawString(Math::Round(y).ToString(), axisFont, textBrush, origin.X - 40, transformed.Y - 10);
                }

                if (isLineGraph)
                {
                    /*
                    * Draw the line graph
                    */
                    for (int i = 1; i < points->Count; i++)
                    {
                        PointF prev = points[i - 1]; // Get point that was previewsly set
                        PointF current = points[i]; // Get the current point

                        PointF prevTransformed((prev.X - xMin) * xScale + drawingArea.Left,
                            drawingArea.Bottom - (prev.Y - yMin) * yScale); // Transform (scale) the previews point 
                        PointF currentTransformed((current.X - xMin) * xScale + drawingArea.Left,
                            drawingArea.Bottom - (current.Y - yMin) * yScale); // Transform (scale) the current point 

                        g->DrawLine(graphPen, prevTransformed, currentTransformed); // Draw the line from previews point to the current point
                    }
                }
                else
                {
                    /*
                    * Draw the column graph
                    */
                    float availableWidth = (drawingArea.Width) / points->Count; // Get available width for each bar
                    float barWidth = availableWidth * 0.01f; // Set the width of the bar (the same with for all of the points)

                    float yZeroPosition = drawingArea.Bottom - (0.0f - yMin) * yScale; // Get the position of 0 on Y axis that the bar would always start Y = 0

                    for each (PointF point in points)
                    {
                        float transformedX = (point.X - xMin) * xScale + drawingArea.Left; // Transform (scale) X position of the bar

                        float topY = (point.Y >= 0)
                            ? yZeroPosition - (point.Y * yScale)
                            : yZeroPosition; // Get the top position of the bar (if the Y > 0 when set to the highest value else set it to the minimum value that is Y = 0)

                        float bottomY = (point.Y >= 0)
                            ? yZeroPosition
                            : yZeroPosition - (point.Y * yScale); // Get the bottom position of the bar (if the Y > 0 when set the bottom position to Y = 0 else set it to the minimum value of Y axis)

                        g->FillRectangle(gcnew SolidBrush(graphColor),
                            transformedX - barWidth / 2.0f,
                            Math::Min(topY, bottomY),
                            barWidth,
                            Math::Abs(topY - bottomY)); // Draw the column from the minimum value to the maximum value
                    }
                }
            }
        }
    };
}