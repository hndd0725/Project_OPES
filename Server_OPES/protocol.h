#pragma once
#include <DirectXMath.h>
using namespace DirectX;

// 기지 체력
constexpr int                  CENTER_HP		= 500000;

// 무기
// DAMAGE 데미지, RPM 연사속도(분당 발사수), RELOAD 재장전 속도, MAGAZINE 장탄수, HIP 견착, ADS 정조준
// 총구 황염 렌더링 시간
constexpr float                FLAME_RENDER_TIME = 0.03f;

// 기관총
constexpr unsigned int         MG_DAMAGE	   = 15;
constexpr float                MG_RECOIL       = 2.5f;
constexpr float				   MG_RECOIL_BACK  = 0.1;
constexpr float                MG_SHOOT_DELAY  = 0.1f;
constexpr float                MG_RELOAD_TIME  = 3.0f;
constexpr unsigned int         MG_MAGAZINE	   = 100;

// 지정 사수 소총
constexpr unsigned int         DMR_DAMAGE	    = 70;
constexpr float                DMR_RECOIL       = 13.0;
constexpr float                DMR_RECOIL_BACK  = 0.2f;
constexpr float                DMR_SHOOT_DELAY  = 0.7f;
constexpr float                DMR_RELOAD_TIME  = 2.0f;
constexpr unsigned int         DMR_MAGAZINE		= 10;

// 샷건
constexpr unsigned int         SG_DAMAGE		= 12;
constexpr float                SG_RECOIL        = 12.0f;
constexpr float                SG_RECOIL_BACK   = 0.2f;
constexpr unsigned int         SG_FRAG			= 12;    // FRAG 파편, PIECE 조각, BUCK 산탄, PELLET 펠릿 용어 아직 못정함.
constexpr float                SG_SHOOT_DELAY   = 1.0f;
constexpr float                SG_RELOAD_TIME	= 2.0f;
constexpr unsigned int         SG_MAGAZINE		= 6;

// 설치 무기
// DAMAGE 데미지, DURABILITY 내구도, INSTALL 설치 시간, RPM 연사속도, COOLDOWN 파괴된 후 쿨타임

// 포탑(TURRET)
constexpr unsigned int         TURRET_ID            = 1;
constexpr unsigned int         TURRET_DAMAGE	    = 10;
constexpr float                TURRET_DURABILITY    = 20.0f;
constexpr float                TURRET_INSTALL_SPEED	= 0.5f;
constexpr float                TURRET_SHOOT_DELAY   = 0.2f;
constexpr float                TURRET_INSTALL_COOLTIME = 20.0f;

constexpr unsigned int         BEACON_ID = 2;
constexpr unsigned int         BEACON_HEAL = 15;
constexpr float                BEACON_DURABILITY = 30.0f;
constexpr float                BEACON_HEAL_DELAY = 0.5f;
constexpr float                BEACON_INSTALL_COOLTIME = 20.0f;

// 캐릭터 LMG 기관총 사수 / DMR 지정사수 / ENG 엔지니어
// HP 체력, SPEED 이동속도(km/h)

//각캐릭터 정보
constexpr int                  CHARACTER_MG = 0;
constexpr int                  CHARACTER_DMR = 1;
constexpr int                  CHARACTER_ENG = 2;

constexpr int				   MG_JOB_TYPE     = 0;
constexpr unsigned int         CHARACTER_MG_HP = 300;
constexpr float                CHARACTER_MG_SPEED   = 11.0f;

constexpr int				   DMR_JOB_TYPE     = 1;
constexpr unsigned int         CHARACTER_DMR_HP = 200;
constexpr float                CHARACTER_DMR_SPEED	= 11.0f;

constexpr int				   ENG_JOB_TYPE      = 2;
constexpr unsigned int         CHARACTER_ENG_HP = 250;
constexpr float                CHARACTER_ENG_SPEED	= 11.0f;

// 몬스터
// HP 체력, SPEED 이동속도(km/h), DAMAGE 데미지, ATTACK 주기(초)

//디팬스 몬스터 수
constexpr int                  DEFENSE_MONSTER1 = 5;
constexpr int                  DEFENSE_MONSTER2 = 5;
constexpr int                  DEFENSE_MONSTER3 = 5;

//start 인원수 
constexpr int				   MIN_PLAYER_COUNT = 3;          

//스테이지 넘어갈때 딜레이 시간
constexpr int                 PASS_STAGE_TIME = 1;

//몬스터 스폰 시간
constexpr int                 CREAT_MONSTER_TIME = 2;

//어드벤처 타이머
constexpr int                 TIMER_ADVENTURE = 300;


// 스테이지 1
// Plant Monster
constexpr unsigned int         PLANT_HP			= 100;
constexpr unsigned int         PLANT_DAMAGE		= 5;
constexpr int                  PLANT_TYPE       = 1;

// Scorpion
constexpr unsigned int         SCORPION_HP		= 300;
constexpr unsigned int         SCORPION_DAMAGE	= 20;
constexpr int                  SCORPION_TYPE    = 2;

// 스테이지 2
// Troll
constexpr unsigned int         TROLL_HP = 200;
constexpr unsigned int         TROLL_DAMAGE = 30;
constexpr int                  TROLL_TYPE = 1;

// Treant
constexpr unsigned int         TREANT_HP = 400;
constexpr unsigned int         TREANT_DAMAGE = 60;
constexpr int                  TREANT_TYPE = 2;

// 스테이지 3
// imp
/* DEPRECATED */
constexpr unsigned int         IMP_HP           = 150;
constexpr unsigned int         IMP_DAMAGE       = 15;
constexpr int                  IMP_TYPE          = 1;
/* DEPRECATED */


// gazer
constexpr unsigned int         GAZER_HP         = 450;
constexpr unsigned int         GAZER_DAMAGE     = 30;
constexpr int                  GAZER_TYPE       = 2;

// 스테이지 1 도착 지점 위치
constexpr XMFLOAT3             MAP1_DESTINATION = XMFLOAT3(120.0, 0.0, 94.0);

// 스테이지1 랜덤 생성 범위
constexpr float                MAP1_RANDOM_MIN_RADIANS = 30.0;
constexpr float                MAP1_RANDOM_MAX_RADIANS = 60.0;

// 스테이지 2 도착 지점 위치
constexpr XMFLOAT3             MAP2_DESTINATION = XMFLOAT3(-210.0, 0.0, 65.0);

// 스테이지 2 랜덤 생성 위치
constexpr float                MAP2_RANDOM_MIN_RADIANS = 40.0;
constexpr float                MAP2_RANDOM_MAX_RADIANS = 70.0;

// 스테이지 3 도착 지점 위치
constexpr XMFLOAT3             MAP3_DESTINATION = XMFLOAT3(275.0, 0.0, 185.0);

// 스테이지 3 랜덤 생성 위치
constexpr float                MAP3_RANDOM_MIN_RADIANS = 150.0;
constexpr float                MAP3_RANDOM_MAX_RADIANS = 180.0;

// 보너스 & 페널티
// 공통 보너스
constexpr float BONUS_RPM = 1.2f;				// 무기 과열 제거 (Weapon Cooling), 무기 연사속도 +20%
constexpr float BONUS_RELOAD = 0.55f;			// 빠른 장전 훈련 (Quick Reload Training), 장전 속도 30% 감소
constexpr int   BONUS_GRENADE = 1;				// 전술 확장 벨트 (Tactical Extended Pocket), 수류탄 +1
constexpr float BONUS_MOVE_SPEED = 1.15f;		// 아드레날린 러시 (Adrenaline Rush), 이동속도 +15%

// 기관총병 보너스
constexpr float BONUS_RECOIL_LMG = 0.6f;		// 반동 제어 숙련 (Recoil Mastery), 반동 -50%
constexpr float BONUS_HP_LMG = 1.3f;			// 굳은 의지 (Strong Will), 체력 +33%

// 저격병 보너스 - 지향/정조준의 차이가 없어 삭제
//constexpr float BONUS_ADS_DAMAGE_DMR = 1.2f;    // 정조준 조준경 (ADS Scope Enhancement), 정조준 데미지 +20%
//constexpr float BONUS_ADS_SPEED_DMR = 1.5f;		// 민첩한 조작 (Agile Handling), 정조준 전환 속도 +50%

// 엔지니어 보너스
constexpr float BONUS_STRUCTURE_DUR_ENG = 1.5f;  // 적응형 구조 (Adaption Structure), 설치 구조물 지속시간 +50% 
//BONUS_STRUCTURE_HP_ENG -> BONUS_STRUCTURE_DUR_ENG HP -> DUR로 변경
constexpr int   BONUS_PELLET_SG = 3;			// 집탄 모듈 확장 (Expanded Pellet Module), 산탄 수 +3

// 공통 페널티
constexpr float PENALTY_RPM = 0.85f;			// 무기 과열 (Weapon Overheating), 무기 연사속도 -15%
constexpr float PENALTY_RELOAD = 1.2f;			// 급한 장전 (Interrupted Reload), 장전 속도 +30%
constexpr float PENALTY_MOVE_SPEED = 0.85f;		// 피로 누적 (Fatigue Accumulation), 이동속도 -15%
constexpr int   PENALTY_GRENADE = -1;			// 탄띠 손상 (Damaged Utility Belt), 수류탄 -2

// 기관총병 페널티
constexpr float PENALTY_RECOIL_LMG = 1.25f;		// 연사 불안정 (Spray Instability), 반동 +25%

// 저격병 페널티 - 지향/정조준의 차이가 없어져 삭제함.
//constexpr float PENALTY_RECOIL_DMR = 1.2f;		// 정조준 불안정 (ADS Instability), 반동 +20%
//constexpr float PENALTY_ADS_RPM_DMR = 0.9f;		// 정조준 불안정 (ADS Instability), 정조준 연사속도 -10%
//constexpr float PENALTY_HIP_RPM_DMR = 0.75f;	// 빠른 지향의 대가 (Hipfire Penalty), 지향사격 연사속도 -25%

// 엔지니어 페널티
constexpr float PENALTY_STRUCTURE_DUR_ENG = 0.7f;// 불안정한 설계 (Flawed Structure), 설치 구조물 지속시간 -30%
//PENALTY_STRUCTURE_HP_ENG -> PENALTY_STRUCTURE_DUR_ENG
constexpr float PENALTY_PELLET_DAMAGE_SG = 0.8f;// 불량 탄약 (Faulty Ammo), 샷건 펠릿당 데미지 -20%

/*
	게임의 긴장감을 위해서 전체적으로 체력을 낮추고자 함.
	MG HP 300 -> 200 / DMR HP 125 -> 100 / SG HP 150 -> 125
	MG DMG 10 -> 12? 기관총병의 DPS 상향 고민

	회복 체감 약함. 5 -> 50

	몬스터 패치
	보너스/페널티로 얻는 수치에 비해 몬스터가 과하게 강해 너프
	Troll HP 250 -> 200 / DMG 25 -> 20
	Treant HP 600 -> 500 / DMG 50 -> 40
	3스테이지는 2스테이지에 비해 낙사가 있지만 감안해도 쉬워 몬스터 추가 배치나 몬스터의 상향이 필요해보임.
	몬스터 배치를 한다면 지금보다 약 2배 많은 것이 좋아보이고
	IMP의 체력을 순간 점사로 죽지 않게 만들고, 데미지를 크게 버프해 긴장감을 부여하는 것이 좋아보임.

	보너스 / 페널티는 각 스테이지가 지나갈 때 마다 +2/-1 or 모든 테이블 중 하나가 적당한 것 같음.
	보너스
	연사속도 20->30%
	장전 속도 30 -> 45%
	수류탄 +2 -> 3

	기관총병 - 반동 -50 -> -40%
	저격병 - 보너스 -> 삭제
	엔지니어 - 적응형 구조 체력 -> 유지 시간

	페널티
	연사속도 -15 -> -10
	이동속도 -15 -> -10

	저격병 - 페널티 삭제
	엔지니어 - 체력 -> 유지시간 / 데미지 -20% -> 펠릿 - 1개


*/