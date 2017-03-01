// ------- imresize_no_scale------- for only 1 channel image, only bilinear, scale can not be used.
// ------- To mimic the Matlab function imresize(), ---------------------------
// ------- Input: could be UINT8, double, int, float. Range [0.0 - 1.0] or [0 - 255]
// ------- target_row or target_col can be 0; Or both target_row and target_col have their non-zero value.
// ------- Can be used for shrinking and enlarging.
int img_processing::imresize_no_scale(Mat img, int target_row, int target_col, bool antialias, Mat *resized_img)
{
	if (target_row == 0 && target_col == 0)
	{
		cout << "Error: both target_row and target_col are 0." << endl;
		return 0;
	}

	if (img.type() != CV_64F)
	{
		Mat img_double;
		img.convertTo(img_double, CV_64FC1);
		img_double.copyTo(img);
	}

	int ori_row = img.rows;
	int ori_col = img.cols;
	double scale;
	Vec2d scale_vec;
	if (target_row == 0)
	{
		scale = double(target_col) / double(ori_col);
		target_row = int(ceil(scale * double(ori_row)));
		scale_vec[0] = scale;
		scale_vec[1] = scale;
	}
	else
	{
		if (target_col == 0)
		{
			scale = double(target_row) / double(ori_row);
			target_col = int(ceil(scale * double(ori_col)));
			scale_vec[0] = scale;
			scale_vec[1] = scale;
		}
		else
		{
			scale_vec[0] = double(target_row) / double(ori_row);
			scale_vec[1] = double(target_col) / double(ori_col);
		}
	}
	Mat weight_height, indices_height;
	Mat weight_width, indices_width;
	int kernel_width = 2;
	int result = imresize_contributions(ori_row, target_row, scale_vec[0], kernel_width, antialias, &weight_height, &indices_height);

	result = imresize_contributions(ori_col, target_col, scale_vec[1], kernel_width, antialias, &weight_width, &indices_width);

	Mat out, out_final;
	Vec2i order;
	if (scale_vec[0] <= scale_vec[1])
	{
		order[0] = 0;
		order[1] = 1;
		result = imresize_resizeAlongDim(img, 0, weight_height, indices_height, &out); // height resize
		result = imresize_resizeAlongDim(out, 1, weight_width, indices_width, &out_final); // width resize
	}
	else
	{
		order[0] = 1;
		order[1] = 0;
		result = imresize_resizeAlongDim(img, 1, weight_width, indices_width, &out); // height resize
		result = imresize_resizeAlongDim(out, 0, weight_height, indices_height, &out_final); // height resize
	}
	if (out_final.empty())
	{
		cout << "Error: image resize errors, out_final is empty." << endl;
		return 0;
	}
	out_final.copyTo(*resized_img);
	return 1;
}

// ------- caculate_kernel_general -------------------------------------------
// ------- To caculate kernel given size and sigma ---------------------------
int img_processing::caculate_kernel_general(double sigma, int kernel_size, Mat *kernel)
{
	double size = (kernel_size - 1.0) / 2.0;

	vector<double> x_row, y_col;
	for (int i = 0; i < kernel_size; i++)
	{
		x_row.push_back(-size + double(i));
		y_col.push_back(-size + double(i));
	}
	Mat x, y;
	repeat(cv::Mat(x_row).t(), kernel_size, 1, x);
	repeat(cv::Mat(y_col), 1, kernel_size, y);

	Mat X_pow2 = x.mul(x);
	Mat Y_pow2 = y.mul(y);
	Mat arg = -(X_pow2 + Y_pow2) / (2 * sigma*sigma);
	Mat h;
	exp(arg, h);
	Mat sum_h;
	reduce(h, sum_h, 1, CV_REDUCE_SUM, -1);
	Scalar sumhh = sum(sum_h);
	if (sumhh[0] != 0.0)
	{
		h = h / sumhh[0];
	}
	else
	{
		return 0;
	}

	h.copyTo(*kernel);
	return 1;
}

// ------- To extract_file_extension -----------------------------
// ------- Input: file name --------------------------------------
// ------- Output: the extension string (not include '.') --------
string img_processing::extract_file_extension(string file_name)
{
	string file_extension;
	std::string::iterator last_point = file_name.end() - 1;
	while (*last_point != '.')
	{
		last_point--;
	}
	last_point++;
	while (last_point != file_name.end())
	{
		file_extension.push_back(*last_point);
		last_point++;
	}

	return file_extension;
}