#pragma once

#include "behaviour/Behaviour.h"

#include <frc/geometry/Transform3d.h>
#include <frc/geometry/Pose3d.h>
#include <photonlib/PhotonCamera.h>
#include <photonlib/SimPhotonCamera.h>
#include <photonlib/RobotPoseEstimator.h>
#include <NTUtil.h>
#include <wpi/json.h>

#include <frc/apriltag/AprilTagFields.h>

#include <frc/apriltag/AprilTagFieldLayout.h>

#include <frc/geometry/Transform3d.h>

using namespace photonlib;

std::shared_ptr<frc::AprilTagFieldLayout> Get2023Layout() {
  return std::make_shared<frc::AprilTagFieldLayout>(frc::LoadAprilTagLayoutField(frc::AprilTagField::k2023ChargedUp));
};

// pp stands for photonpipeline in variable naming

struct VisionConfig {
  std::shared_ptr<PhotonCamera> camera;

  units::radian_t fov;
  frc::Transform3d robotToCamera; // assuming this means centre of robot to camera, but UNSURE
  std::shared_ptr<frc::AprilTagFieldLayout> layout;


};

class Vision {
  private :
    VisionConfig defaultConfig;
    VisionConfig visionConfig;
    RobotPoseEstimator _estimator;
  public :
    // Vision(VisionConfig config, RobotPoseEstimator estimator);
    Vision(VisionConfig config);
    
    void OnUpdate(units::second_t dt); 

    PhotonPipelineResult getLatestResults(std::shared_ptr<PhotonCamera> camera) {
      PhotonPipelineResult ppResult = camera->GetLatestResult();
      return ppResult;
    };

    auto getTargets(std::shared_ptr<PhotonCamera> camera) {
      PhotonPipelineResult ppResults = getLatestResults(visionConfig.camera);
      auto targets = ppResults.GetTargets();
      return targets;
    };

  PhotonTrackedTarget getBestTarget(std::shared_ptr<PhotonCamera> camera, PhotonPipelineResult result) {
    PhotonTrackedTarget bestTarget = result.GetBestTarget();
    return bestTarget;
  };

  auto estimatePose(VisionConfig config) {
  
    visionConfig = config;
    _estimator = RobotPoseEstimator{
      visionConfig.layout,
      photonlib::AVERAGE_BEST_TARGETS,
      {std::make_pair(config.camera, config.robotToCamera)}
    };

    std::pair<frc::Pose3d, units::millisecond_t> pose_result = _estimator.Update();
    auto table = nt::NetworkTableInstance::GetDefault().GetTable("Vision");
    wom::WritePose3NT(table, pose_result.first);
    return pose_result.first;
  };

  auto getPathForBest(std::shared_ptr<PhotonCamera> camera) {
    PhotonPipelineResult ppResults = getLatestResults(camera);
    PhotonTrackedTarget bestTarget = getBestTarget(camera, ppResults);
    frc::Pose3d poseEstimate = estimatePose(defaultConfig);
    frc::Transform3d relativeBestTargetPose = bestTarget.GetBestCameraToTarget(); // relative position of target (treating robot position as (0, 0, 0, 0))
    frc::Pose2d bestTargetPose = frc::Pose2d{
      frc::Translation2d{poseEstimate.X() + relativeBestTargetPose.X(), poseEstimate.Y() + relativeBestTargetPose.Y()},
      poseEstimate.Rotation().ToRotation2d()
    };

    return bestTargetPose;

  };

  // class getTargetBehaviour : public behaviour::Behaviour{
  //   public:
  //     getTargetBehaviour(wom::SwerveDrive *swerveDrivebase, frc::Pose2d pose);
  //     void OnTick(units::second_t deltaTime) override;

  //   private:
  //     wom::SwerveDrive *_swerveDrivebase;
  //     frc::Pose2d _pose; // storing it directly, so not a pointer
  // };

};

  // //new other things
  // PhotonPipelineResult ppResult = camera.GetLatestResult();

  // bool hasTargets = ppResult.HasTargets();

  // //PhotonTrackedTarget targets = ppResult.GetTargets();

  // auto targets = ppResult.GetTargets();
  
  // PhotonTrackedTarget target = ppResult.GetBestTarget();



std::shared_ptr<frc::AprilTagFieldLayout> Get2023Layout();