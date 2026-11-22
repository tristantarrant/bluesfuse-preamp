// generated from file 'src/bluesfuse.dsp' by dsp2cc:
// Code generated with Faust 2.85.9 (https://faust.grame.fr)


namespace bluesfuse {

class Dsp: public PluginLV2 {
private:
	uint32_t fSampleRate;
	double fConst0;
	double fConst1;
	double fConst2;
	double fConst3;
	FAUSTFLOAT fEntry0;
	FAUSTFLOAT	*fEntry0_;
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT	*fHslider0_;
	double fRec1[2];
	FAUSTFLOAT fHslider1;
	FAUSTFLOAT	*fHslider1_;
	double fRec2[2];
	double fConst4;
	double fConst5;
	double fConst6;
	double fConst7;
	double fConst8;
	double fConst9;
	double fConst10;
	FAUSTFLOAT fHslider2;
	FAUSTFLOAT	*fHslider2_;
	double fRec4[2];
	FAUSTFLOAT fHslider3;
	FAUSTFLOAT	*fHslider3_;
	double fRec6[2];
	double fConst11;
	double fConst12;
	FAUSTFLOAT fEntry1;
	FAUSTFLOAT	*fEntry1_;
	FAUSTFLOAT fHslider4;
	FAUSTFLOAT	*fHslider4_;
	double fRec8[2];
	double fConst13;
	double fVec0[2];
	double fConst14;
	double fConst15;
	double fConst16;
	double fRec11[2];
	double fRec10[2];
	double fRec9[3];
	double fRec7[4];
	double fRec5[3];
	double fRec3[5];
	double fVec1[2];
	double fConst17;
	double fConst18;
	double fRec0[2];

	void connect(uint32_t port,void* data);
	void clear_state_f();
	void init(uint32_t sample_rate);
	void compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0);

	static void clear_state_f_static(PluginLV2*);
	static void init_static(uint32_t sample_rate, PluginLV2*);
	static void compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginLV2*);
	static void del_instance(PluginLV2 *p);
	static void connect_static(uint32_t port,void* data, PluginLV2 *p);
public:
	Dsp();
	~Dsp();
};



Dsp::Dsp()
	: PluginLV2() {
	version = PLUGINLV2_VERSION;
	id = "bluesfuse";
	name = N_("BluesFuse Preamp");
	mono_audio = compute_static;
	stereo_audio = 0;
	set_samplerate = init_static;
	activate_plugin = 0;
	connect_ports = connect_static;
	clear_state = clear_state_f_static;
	delete_instance = del_instance;
}

Dsp::~Dsp() {
}

inline void Dsp::clear_state_f()
{
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec1[l0] = 0.0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec2[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec4[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec6[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec8[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fVec0[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fRec11[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec10[l7] = 0.0;
	for (int l8 = 0; l8 < 3; l8 = l8 + 1) fRec9[l8] = 0.0;
	for (int l9 = 0; l9 < 4; l9 = l9 + 1) fRec7[l9] = 0.0;
	for (int l10 = 0; l10 < 3; l10 = l10 + 1) fRec5[l10] = 0.0;
	for (int l11 = 0; l11 < 5; l11 = l11 + 1) fRec3[l11] = 0.0;
	for (int l12 = 0; l12 < 2; l12 = l12 + 1) fVec1[l12] = 0.0;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) fRec0[l13] = 0.0;
}

void Dsp::clear_state_f_static(PluginLV2 *p)
{
	static_cast<Dsp*>(p)->clear_state_f();
}

inline void Dsp::init(uint32_t sample_rate)
{
	fSampleRate = sample_rate;
	fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, static_cast<double>(fSampleRate)));
	fConst1 = std::tan(10.638297872340425 / fConst0);
	fConst2 = 1.0 / fConst1;
	fConst3 = 1.0 - fConst2;
	fConst4 = mydsp_faustpower3_f(fConst0);
	fConst5 = 16.0 * fConst4;
	fConst6 = mydsp_faustpower2_f(fConst0);
	fConst7 = 8.0 * fConst6;
	fConst8 = 4.0 * fConst0;
	fConst9 = 64.0 * fConst0;
	fConst10 = 96.0 * fConst6;
	fConst11 = 2.0 * fConst0;
	fConst12 = 24.0 * fConst6;
	fConst13 = 4.9999999999999996e-05 / fConst0;
	fConst14 = 1.0 / std::tan(48.25323296660877 / fConst0);
	fConst15 = 1.0 - fConst14;
	fConst16 = 1.0 / (fConst14 + 1.0);
	fConst17 = 0.1 / fConst1;
	fConst18 = 1.0 / (fConst2 + 1.0);
	clear_state_f();
}

void Dsp::init_static(uint32_t sample_rate, PluginLV2 *p)
{
	static_cast<Dsp*>(p)->init(sample_rate);
}

void always_inline Dsp::compute(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0)
{
#define fEntry0 (*fEntry0_)
#define fHslider0 (*fHslider0_)
#define fHslider1 (*fHslider1_)
#define fHslider2 (*fHslider2_)
#define fHslider3 (*fHslider3_)
#define fEntry1 (*fEntry1_)
#define fHslider4 (*fHslider4_)
	int iSlow0 = static_cast<int>(static_cast<double>(fEntry0));
	double fSlow1 = 0.0010000000000000009 * static_cast<double>(fHslider0);
	double fSlow2 = 0.0010000000000000009 * static_cast<double>(fHslider1);
	double fSlow3 = 0.0010000000000000009 * static_cast<double>(fHslider2);
	double fSlow4 = 0.0010000000000000009 * static_cast<double>(fHslider3);
	double fSlow5 = ((iSlow0) ? 1.9697785938008004e+21 : -1.0368292482422002e+24);
	double fSlow6 = ((iSlow0) ? 1.790707812546182e+21 : -9.425720438565457e+23);
	double fSlow7 = 3.0 * fSlow5;
	double fSlow8 = static_cast<double>(fEntry1);
	int iSlow9 = fSlow8 == 1.0;
	double fSlow10 = static_cast<double>(((fSlow8 == 0.0) ? 1 : 2));
	double fSlow11 = 0.0010000000000000009 * static_cast<double>(fHslider4);
	double fSlow12 = ((iSlow0) ? 1e+18 : 1e+28);
	double fSlow13 = 2.0 * fSlow12;
	double fSlow14 = std::tan(fConst13 / ((iSlow0) ? 1e-07 : 1e-06));
	double fSlow15 = 1.0 / fSlow14;
	double fSlow16 = fSlow15 + 1.0;
	double fSlow17 = 1.0 - fSlow15;
	double fSlow18 = ((iSlow0) ? 4.7e+03 : 1e+04);
	double fSlow19 = 3.0 * fSlow6;
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		fRec1[0] = fSlow1 + 0.999 * fRec1[1];
		double fTemp0 = 1.4133188533117502e+21 * fRec1[0];
		double fTemp1 = 7.0664529375e+25 * fRec1[0];
		fRec2[0] = fSlow2 + 0.999 * fRec2[1];
		double fTemp2 = 6.3980029575e+17 * fRec1[0];
		double fTemp3 = 3.1989375000000005e+22 * fRec1[0];
		double fTemp4 = ((iSlow0) ? 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (0.1 * fRec1[0] * (fTemp3 + 1.324360125e+23) + -5.163265162482075e+23) + fRec1[0] * (-1.324360125e+23 - fTemp3)) + 5.163265162482075e+23) + fRec1[0] * (-2.648773224405e+18 - fTemp2)) + 1.032673685557065e+19 : 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (0.1 * fRec1[0] * (fTemp1 + 2.925511516125e+26) + -1.1405652743922904e+27) + fRec1[0] * (-2.925511516125e+26 - fTemp1)) + 1.1405652743922904e+27) + fRec1[0] * (-5.851140052710645e+21 - fTemp0)) + 2.2811761713955566e+22);
		double fTemp5 = fConst5 * fTemp4;
		double fTemp6 = 4.28378128867425e+28 * fRec1[0];
		double fTemp7 = 4.556062500000001e+29 * fRec1[0];
		double fTemp8 = 1.93924005825e+25 * fRec1[0];
		double fTemp9 = 2.0625000000000002e+26 * fRec1[0];
		double fTemp10 = ((iSlow0) ? 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (0.1 * fRec1[0] * (fTemp9 + 1.04775e+27) + -5.323257572325e+27) + fRec1[0] * (-1.04775e+27 - fTemp9)) + 5.323257572325e+27) + fRec1[0] * (-8.02884159891e+25 - fTemp8)) + 3.13044145649754e+26 : 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (0.1 * fRec1[0] * (fTemp7 + 2.31447975e+30) + -1.6243841529967425e+31) + fRec1[0] * (-2.31447975e+30 - fTemp7)) + 1.6243841529967425e+31) + fRec1[0] * (-1.773571109199219e+29 - fTemp6)) + 6.916042148452668e+29);
		double fTemp11 = fConst7 * fTemp10;
		double fTemp12 = 2.7612500000000005e+32 * fRec1[0];
		double fTemp13 = fRec1[0] * (fTemp12 + -5.5225e+33);
		double fTemp14 = 1.25e+29 * fRec1[0];
		double fTemp15 = fRec1[0] * (fTemp14 + -2.5e+30);
		double fTemp16 = ((iSlow0) ? 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (fTemp14 + -1.4525265e+31) + fTemp15) + 1.42752299995e+31) + fRec1[0] * (2.588172995e+29 - 1.367525e+29 * fRec1[0])) + 2.473726331771516e+30 : 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (fTemp12 + -7.1936680725e+34) + fTemp13) + 7.13843534088955e+34) + fRec1[0] * (5.717274145955e+32 - 3.0208627250000003e+32 * fRec1[0])) + 8.183406940729383e+33);
		double fTemp17 = fConst8 * fTemp16;
		double fTemp18 = 9.425e+36 * fRec2[0];
		double fTemp19 = 1.075e+33 * fRec2[0];
		double fTemp20 = ((iSlow0) ? 0.1 * fRec2[0] * (7.2499e+33 - fTemp19) + 6.8222908263e+33 : 0.1 * fRec2[0] * (7.779953e+37 - fTemp18) + 3.61759558071e+37);
		double fTemp21 = 2.0 * fTemp20;
		double fTemp22 = 5e+39 * fRec2[0];
		double fTemp23 = 5e+35 * fRec2[0];
		double fTemp24 = ((iSlow0) ? 5.4701e+36 - fTemp23 : 5.4701e+40 - fTemp22);
		double fTemp25 = fTemp24 + fConst0 * (fTemp21 + fTemp17 + fTemp11 + fTemp5);
		double fTemp26 = ((iSlow0) ? 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (0.1 * fRec1[0] * (5.17275e+27 - fTemp9) + 2.213007572325e+27) + fRec1[0] * (fTemp9 + -5.17275e+27)) + -2.213007572325e+27) + fRec1[0] * (fTemp8 + -4.681364276391e+26)) + -3.8831723835654034e+25 : 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (0.1 * fRec1[0] * (1.142660475e+31 - fTemp7) + 9.373299279967425e+30) + fRec1[0] * (fTemp7 + -1.142660475e+31)) + -9.373299279967425e+30) + fRec1[0] * (fTemp6 + -1.034113368654772e+30)) + -8.586897505792e+28);
		double fTemp27 = fConst7 * fTemp26;
		double fTemp28 = ((iSlow0) ? 0.1 * fRec2[0] * (fTemp19 + -1.42501e+34) + -3.3221908263e+33 : 0.1 * fRec2[0] * (fTemp18 + -1.1070047e+38) + -1.97254858071e+37);
		double fTemp29 = 2.0 * fTemp28;
		double fTemp30 = ((iSlow0) ? 0.1 * (fRec2[0] * (0.1 * (fTemp15 + fRec2[0] * (fTemp14 + 1.3275265e+31)) + -1.35253000005e+31) + fRec1[0] * (1.17525e+28 * fRec1[0] + -1.2262327005e+30)) + -1.114983630771516e+30 : 0.1 * (fRec2[0] * (0.1 * (fTemp13 + fRec2[0] * (fTemp12 + 6.9175430725e+34)) + -6.97277580411045e+34) + fRec1[0] * (2.59612725e+31 * fRec1[0] + -2.7087480354045e+33)) + -5.181944314220383e+33);
		double fTemp31 = fConst8 * fTemp30;
		double fTemp32 = ((iSlow0) ? 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (0.1 * fRec1[0] * (7.722235125e+23 - fTemp3) + 6.39967537482075e+22) + fRec1[0] * (fTemp3 + -7.722235125e+23)) + -6.39967537482075e+22) + fRec1[0] * (fTemp2 + -1.5444779139405e+19)) + -1.2799606736656494e+18 : 0.1 * (fRec2[0] * (0.1 * (fRec2[0] * (0.1 * fRec1[0] * (1.7058417391125e+27 - fTemp1) + 1.4136882902979036e+26) + fRec1[0] * (fTemp1 + -1.7058417391125e+27)) + -1.4136882902979036e+26) + fRec1[0] * (fTemp0 + -3.4117517118945645e+22)) + -2.827433128127419e+21);
		double fTemp33 = fConst5 * fTemp32;
		double fTemp34 = ((iSlow0) ? -4.701e+35 - fTemp23 : -4.701e+39 - fTemp22);
		double fTemp35 = fConst9 * fTemp4;
		double fTemp36 = 16.0 * fTemp10;
		double fTemp37 = fConst0 * fTemp20;
		fRec4[0] = fSlow3 + 0.999 * fRec4[1];
		double fTemp38 = ((iSlow0) ? 4.653e+07 * fRec4[0] + -499426653.0 : 4.089e+07 * fRec4[0] + -445352589.0);
		double fTemp39 = fConst8 * fTemp38;
		double fTemp40 = 2.0 * ((iSlow0) ? 6.601551e+12 * fRec4[0] + -7.08595823551e+13 : 5.807755e+12 * fRec4[0] + -6.32845792755e+13);
		double fTemp41 = ((iSlow0) ? 2.2e+14 * fRec4[0] + -2.684022e+15 : 1.1e+15 * fRec4[0] + -1.617011e+16);
		double fTemp42 = fTemp41 + fConst0 * (fTemp40 + fTemp39);
		fRec6[0] = fSlow4 + 0.999 * fRec6[1];
		double fTemp43 = 0.1 * fRec6[0];
		double fTemp44 = ((fTemp43 > 0.5) ? 1.0 - 0.5 * std::pow(2.0 * (1.0 - fTemp43), 2.51294159473206) : 0.5 * std::pow(0.2 * fRec6[0], 2.51294159473206));
		double fTemp45 = ((iSlow0) ? 95001.0 : fTemp44 * (4.84e+10 * fTemp44 + -5.082e+10) + -561614361.0);
		double fTemp46 = fConst7 * fTemp45;
		double fTemp47 = fConst8 * ((iSlow0) ? 105353370392.3062 : fTemp44 * (2.42e+14 * fTemp44 + -2.431e+14 - 9.123185066024759e+17 * (1.0 - fTemp44)) + -10108156050915888.0);
		double fTemp48 = 2.0 * ((iSlow0) ? 3.5627007669567636e+18 : 3.141592653589793 * fTemp44 * (1.452e+21 * fTemp44 + -1.386e+21) + -4.651922141866777e+20);
		double fTemp49 = fTemp48 + fTemp47;
		double fTemp50 = fSlow5 + fConst0 * (fTemp49 + fTemp46);
		double fTemp51 = fConst11 * ((iSlow0) ? 1.7907257196243077e+18 : 3.141592653589793 * fTemp44 * (1.32e+21 * fTemp44 + -1.386e+21) + -4.583820966080634e+19);
		double fTemp52 = fConst12 * fTemp45;
		fRec8[0] = fSlow11 + 0.999 * fRec8[1];
		double fTemp53 = ((iSlow0) ? 4.7e+11 * fRec8[0] + 2.20947e+10 : 1.03823e+19 * fRec8[0] + 1.038333823e+18);
		double fTemp54 = fConst8 * fTemp53;
		double fTemp55 = 2.0 * ((iSlow0) ? 4.7e+12 * fRec8[0] + 1.00000220947e+17 : 4.7e+22 * fRec8[0] + 2.21370047e+24);
		double fTemp56 = fSlow12 + fConst0 * (fTemp55 + fTemp54);
		double fTemp57 = static_cast<double>(input0[i0]);
		fVec0[0] = fTemp57;
		fRec11[0] = -(fConst16 * (fConst15 * fRec11[1] - fConst14 * (fTemp57 - fVec0[1])));
		fRec10[0] = -((fRec10[1] * fSlow17 - (fRec11[0] - fRec11[1]) / fSlow14) / fSlow16);
		fRec9[0] = fRec10[0] - (fRec9[1] * (fSlow13 - fConst7 * fTemp53) + fRec9[2] * (fSlow12 + fConst0 * (fTemp54 - fTemp55))) / fTemp56;
		double fTemp58 = fConst11 * (((iSlow0) ? 1e+18 * fRec8[0] + 4.701e+16 : 4.7e+25 * fRec8[0] + 4.70047e+24) * (fRec9[0] - fRec9[2]) / fTemp56);
		double fTemp59 = std::fabs(fTemp58);
		double fTemp60 = fSlow18 + 1e+05 * fRec8[0] + 1.0;
		double fTemp61 = static_cast<double>((fTemp58 > 0.0) - (fTemp58 < 0.0)) * std::min<double>(fTemp59, 0.0452892 * fSlow10 * asinh(198412698.41269842 * (fTemp59 / fTemp60)));
		double fTemp62 = std::exp(22.080319369739364 * (fTemp61 / fSlow10));
		double fTemp63 = 1.0 / fTemp62;
		double fTemp64 = (fTemp61 + 2.52e-09 * fTemp60 * (fTemp62 - fTemp63) - fTemp58) / (5.564240481174319e-08 * (fTemp60 * (fTemp62 + fTemp63) / fSlow10) + 1.0);
		double fTemp65 = std::exp(22.080319369739364 * ((fTemp61 - fTemp64) / fSlow10));
		double fTemp66 = 1.0 / fTemp65;
		double fTemp67 = fTemp58 + fTemp64;
		double fTemp68 = (fTemp61 + 2.52e-09 * fTemp60 * (fTemp65 - fTemp66) - fTemp67) / (5.564240481174319e-08 * (fTemp60 * (fTemp65 + fTemp66) / fSlow10) + 1.0);
		double fTemp69 = fTemp64 + fTemp68;
		double fTemp70 = std::exp(22.080319369739364 * ((fTemp61 - fTemp69) / fSlow10));
		double fTemp71 = 1.0 / fTemp70;
		double fTemp72 = fTemp67 + fTemp68;
		double fTemp73 = (fTemp61 + 2.52e-09 * fTemp60 * (fTemp70 - fTemp71) - fTemp72) / (5.564240481174319e-08 * (fTemp60 * (fTemp70 + fTemp71) / fSlow10) + 1.0);
		double fTemp74 = fTemp69 + fTemp73;
		double fTemp75 = std::exp(22.080319369739364 * ((fTemp61 - fTemp74) / fSlow10));
		double fTemp76 = 1.0 / fTemp75;
		double fTemp77 = fRec10[0] + ((iSlow9) ? fTemp58 : fTemp61 - (fTemp74 + (fTemp61 + 2.52e-09 * fTemp60 * (fTemp75 - fTemp76) - (fTemp72 + fTemp73)) / (5.564240481174319e-08 * (fTemp60 * (fTemp75 + fTemp76) / fSlow10) + 1.0)));
		fRec7[0] = fTemp77 / std::sqrt(std::sqrt(mydsp_faustpower4_f(0.2777777777777778 * std::fabs(fTemp77)) + 1.0)) - (fRec7[1] * (fSlow7 + fConst0 * (fTemp48 - fTemp47 - fTemp52)) + fRec7[2] * (fSlow7 + fConst0 * (fTemp52 - fTemp49)) + fRec7[3] * (fSlow5 + fConst0 * (fTemp47 - fTemp48 - fTemp46))) / fTemp50;
		double fTemp78 = fRec7[0] * (fSlow6 + fTemp51) + fRec7[1] * (fSlow19 + fTemp51) + fRec7[2] * (fSlow19 - fTemp51) + fRec7[3] * (fSlow6 - fTemp51);
		fRec5[0] = fTemp78 / (fTemp50 * std::sqrt(std::sqrt(mydsp_faustpower4_f(0.2777777777777778 * std::fabs(fTemp78 / fTemp50)) + 1.0))) - (fRec5[1] * (2.0 * fTemp41 - fConst7 * fTemp38) + fRec5[2] * (fTemp41 + fConst0 * (fTemp39 - fTemp40))) / fTemp42;
		double fTemp79 = ((iSlow0) ? 1.1468094e+14 - 9.4e+12 * fRec4[0] : 6.909047e+13 - 4.7e+12 * fRec4[0]) * (fRec5[0] - fRec5[2]);
		fRec3[0] = fConst11 * (fTemp79 / (fTemp42 * std::sqrt(std::sqrt(mydsp_faustpower4_f(0.2777777777777778 * std::fabs(fConst11 * (fTemp79 / fTemp42))) + 1.0)))) - (fRec3[1] * (4.0 * (fTemp24 + fTemp37) - fConst4 * (fTemp36 + fTemp35)) + fRec3[2] * (6.0 * fTemp24 + fConst6 * (fConst10 * fTemp4 - 8.0 * fTemp16)) + fRec3[3] * (4.0 * (fTemp24 - fTemp37) + fConst4 * (fTemp36 - fTemp35)) + fRec3[4] * (fTemp24 + fConst0 * (fTemp5 + (fTemp17 - fTemp21 - fTemp11)))) / fTemp25;
		double fTemp80 = fConst9 * fTemp32;
		double fTemp81 = 16.0 * fTemp26;
		double fTemp82 = fConst0 * fTemp28;
		double fTemp83 = fRec3[0] * (fTemp34 + fConst0 * (fTemp29 + fTemp31 + fTemp27 + fTemp33)) + fRec3[1] * (4.0 * (fTemp34 + fTemp82) - fConst4 * (fTemp81 + fTemp80)) + fRec3[2] * (6.0 * fTemp34 + fConst6 * (fConst10 * fTemp32 - 8.0 * fTemp30)) + fRec3[3] * (4.0 * (fTemp34 - fTemp82) + fConst4 * (fTemp81 - fTemp80)) + fRec3[4] * (fTemp34 + fConst0 * (fTemp33 + (fTemp31 - fTemp29 - fTemp27)));
		double fTemp84 = fRec4[0] * fTemp83 / (fTemp25 * std::sqrt(std::sqrt(mydsp_faustpower4_f(0.2777777777777778 * std::fabs(fTemp83 / fTemp25)) + 1.0)));
		fVec1[0] = fTemp84;
		fRec0[0] = fConst18 * (fConst17 * (fTemp84 - fVec1[1]) - fConst3 * fRec0[1]);
		output0[i0] = static_cast<FAUSTFLOAT>(fRec0[0]);
		fRec1[1] = fRec1[0];
		fRec2[1] = fRec2[0];
		fRec4[1] = fRec4[0];
		fRec6[1] = fRec6[0];
		fRec8[1] = fRec8[0];
		fVec0[1] = fVec0[0];
		fRec11[1] = fRec11[0];
		fRec10[1] = fRec10[0];
		fRec9[2] = fRec9[1];
		fRec9[1] = fRec9[0];
		for (int j0 = 3; j0 > 0; j0 = j0 - 1) {
			fRec7[j0] = fRec7[j0 - 1];
		}
		fRec5[2] = fRec5[1];
		fRec5[1] = fRec5[0];
		for (int j1 = 4; j1 > 0; j1 = j1 - 1) {
			fRec3[j1] = fRec3[j1 - 1];
		}
		fVec1[1] = fVec1[0];
		fRec0[1] = fRec0[0];
	}
#undef fEntry0
#undef fHslider0
#undef fHslider1
#undef fHslider2
#undef fHslider3
#undef fEntry1
#undef fHslider4
}

void __rt_func Dsp::compute_static(int count, FAUSTFLOAT *input0, FAUSTFLOAT *output0, PluginLV2 *p)
{
	static_cast<Dsp*>(p)->compute(count, input0, output0);
}


void Dsp::connect(uint32_t port,void* data)
{
	switch ((PortIndex)port)
	{
	case BASS: 
		fHslider1_ = (float*)data; // , 5.0, 0.0, 1e+01, 0.1 
		break;
	case GAIN: 
		fHslider4_ = (float*)data; // , 5.0, 0.0, 1e+01, 0.1 
		break;
	case MID: 
		fHslider3_ = (float*)data; // , 2.0, 0.0, 1e+01, 0.1 
		break;
	case TREBLE: 
		fHslider0_ = (float*)data; // , 5.0, 0.0, 1e+01, 0.1 
		break;
	case VOLUME: 
		fHslider2_ = (float*)data; // , 5.0, 0.0, 1e+01, 0.1 
		break;
	case COMP: 
		fEntry1_ = (float*)data; // , 2.0, 0.0, 2.0, 1.0 
		break;
	case MODEL: 
		fEntry0_ = (float*)data; // , 0.0, 0.0, 1.0, 1.0 
		break;
	default:
		break;
	}
}

void Dsp::connect_static(uint32_t port,void* data, PluginLV2 *p)
{
	static_cast<Dsp*>(p)->connect(port, data);
}


PluginLV2 *plugin() {
	return new Dsp();
}

void Dsp::del_instance(PluginLV2 *p)
{
	delete static_cast<Dsp*>(p);
}

/*
typedef enum
{
   BASS, 
   COMP, 
   GAIN, 
   MID, 
   MODEL, 
   TREBLE, 
   VOLUME, 
} PortIndex;
*/

} // end namespace bluesfuse
